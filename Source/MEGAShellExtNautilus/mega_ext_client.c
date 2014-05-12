#include "mega_ext_client.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

const gchar OP_PATH_STATE  = 'P'; //Path state
const gchar OP_INIT        = 'I'; //Init operation
const gchar OP_END         = 'E'; //End operation
const gchar OP_UPLOAD      = 'F'; //File-Folder upload
const gchar OP_LINK        = 'L'; //paste Link
const gchar OP_SHARE       = 'S'; //Share folder
const gchar OP_SEND        = 'C'; //Copy to user
const gchar OP_STRING      = 'T'; //Get Translated String

static void mega_ext_client_disconnect(MEGAExt *mega_ext);

// try to connect to the server
// return TRUE if connection established
static gboolean mega_ext_client_reconnect(MEGAExt *mega_ext)
{
    int len;
    struct sockaddr_un remote;
    gchar *sock_path;
    const gchar sock_file[] = "mega.socket";
    // XXX: current path MEGASync uses to store private data
    const gchar sock_path_hardcode[] = ".local/share/data/Mega Limited/MEGAsync";

    if ((mega_ext->srv_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        g_warning("socket() failed");
        goto failed;
    }

    sock_path = g_build_filename(g_get_home_dir(), sock_path_hardcode, sock_file, NULL);

    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, sock_path, sizeof(remote.sun_path));
    g_free(sock_path);

    g_debug("Connecting to: %s", remote.sun_path);

    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(mega_ext->srv_sock, (struct sockaddr *)&remote, len) == -1) {
        g_warning("connect() failed");
        goto failed;
    }
    g_debug("Connected to the server!");

    mega_ext->chan = g_io_channel_unix_new(mega_ext->srv_sock);
    if (!mega_ext->chan) {
        g_warning("g_io_channel_unix_new() failed");
        goto failed;
    }
    g_io_channel_set_close_on_unref(mega_ext->chan, TRUE);
    g_io_channel_set_line_term(mega_ext->chan, "\n", -1);

    return TRUE;

failed:
    mega_ext_client_disconnect(mega_ext);
    return FALSE;
}

// disconnect client
static void mega_ext_client_disconnect(MEGAExt *mega_ext)
{
    g_debug("Client disconnected");

    if (mega_ext->chan) {
        g_io_channel_shutdown(mega_ext->chan, FALSE, NULL);
        g_io_channel_unref(mega_ext->chan);
        mega_ext->chan = NULL;
    }

    if (mega_ext->srv_sock > 0)
        close(mega_ext->srv_sock);
    mega_ext->srv_sock = -1;
}

// send request and receive response from Extension server
// Return newly-allocated response string
static gchar *mega_ext_client_send_request(MEGAExt *mega_ext, gchar type, const gchar *in)
{
    gchar *out = NULL;
    gchar *tmp;
    gsize bytes_written;
    GError *error;
    GIOStatus status;
    gint num_retries;

    g_debug("Sending request: %s ", in);

    // try to send request several times
    for (num_retries = 0; num_retries < mega_ext->num_retries; num_retries++) {
        if (mega_ext->srv_sock < 0) {
            if (!mega_ext_client_reconnect(mega_ext)) {
                g_debug("Failed to reconnect!");
                continue;
            }
        }

        // format request string
        tmp = g_strdup_printf("%c:%s", type, in);

        error = NULL;
        // try to send request
        status = g_io_channel_write_chars(mega_ext->chan, tmp, strlen(tmp), &bytes_written, &error);
        if (status != G_IO_STATUS_NORMAL || error) {
            g_warning("Failed to write data!");
            g_free(tmp);
            mega_ext_client_disconnect(mega_ext);
            continue;
        }
        g_free(tmp);

        status = g_io_channel_flush(mega_ext->chan, &error);
        if (status != G_IO_STATUS_NORMAL || error) {
            g_debug("Failed to flush data!");
            mega_ext_client_disconnect(mega_ext);
            continue;
        }

        // try to read response
        status = g_io_channel_read_line(mega_ext->chan, &out, NULL, NULL, &error);
        if (status != G_IO_STATUS_NORMAL || error) {
            g_warning("Failed to read data!");
            if (out)
                g_free(out);
            mega_ext_client_disconnect(mega_ext);
            continue;
        }
        break;
    }

    if (!out)
        return NULL;

    // remove last character if it's a carriage return
    if (strlen(out) > 1 && out[strlen(out)-1] == '\n')
        out[strlen(out)-1] = '\0';

    return out;
}

// return a newly-allocated string
gchar *mega_ext_client_get_string(MEGAExt *mega_ext, int stringID, int numFiles, int numFolders)
{
    gchar *in;
    gchar *out;

    in = g_strdup_printf("%d:%d:%d", stringID, numFiles, numFolders);
    out = mega_ext_client_send_request(mega_ext, OP_STRING, in);
    g_free(in);

    return out;
}

FileState mega_ext_client_get_path_state(MEGAExt *mega_ext, const gchar *path)
{
    gchar *out;
    FileState st;

    out = mega_ext_client_send_request(mega_ext, OP_PATH_STATE, path);

    if (!out)
        return FILE_ERROR;

    st = out[0]-'0';
    g_free(out);

    return st;
}

gboolean mega_ext_client_paste_link(MEGAExt *mega_ext, const gchar *path)
{
    gchar *out;

    out = mega_ext_client_send_request(mega_ext, OP_LINK, path);

    if (!out)
        return FALSE;
    g_free(out);

    return TRUE;
}

gboolean mega_ext_client_upload(MEGAExt *mega_ext, const gchar *path)
{
    gchar *out;

    out = mega_ext_client_send_request(mega_ext, OP_UPLOAD, path);

    if (!out)
        return FALSE;
    g_free(out);

    return TRUE;
}

gboolean mega_ext_client_end_request(MEGAExt *mega_ext)
{
    gchar *out;

    out = mega_ext_client_send_request(mega_ext, OP_END, "");

    if (!out)
        return FALSE;
    g_free(out);

    return TRUE;
}
