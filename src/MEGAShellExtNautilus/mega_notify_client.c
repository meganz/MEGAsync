#include "mega_notify_client.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static gboolean mega_notify_client_read(GIOChannel *notify_chan, GIOCondition condition, gpointer data);
static gboolean mega_notify_client_try_connect(MEGAExt *mega_ext);

// return FALSE to stop timer
static gboolean mega_notify_client_on_timer(gpointer user_data)
{
    MEGAExt *mega_ext = (MEGAExt *)user_data;

    return !mega_notify_client_try_connect(mega_ext);
}

void mega_notify_client_timer_start(MEGAExt *mega_ext)
{
    g_debug("Starting timer");
    g_timeout_add_seconds(1, mega_notify_client_on_timer, mega_ext);
}

// try to connect to MEGASync notify server
// return TRUE if connection is established
static gboolean mega_notify_client_try_connect(MEGAExt *mega_ext)
{
    int len;
    struct sockaddr_un remote;
    gchar *sock_path;
    const gchar sock_file[] = "notify.socket";
    // XXX: current path MEGASync uses to store private data
    const gchar sock_path_hardcode[] = ".local/share/data/Mega Limited/MEGAsync";

    if ((mega_ext->notify_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        g_warning("socket() failed: %s", strerror(errno));
        mega_notify_client_destroy(mega_ext);
        return FALSE;
    }

    sock_path = g_build_filename(g_get_home_dir(), sock_path_hardcode, sock_file, NULL);

    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, sock_path, sizeof(remote.sun_path));
    g_free(sock_path);

    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(mega_ext->notify_sock, (struct sockaddr *)&remote, len) == -1) {
        g_warning("connect() failed");
        mega_notify_client_destroy(mega_ext);
        return FALSE;
    }
    g_debug("Connected to notify server!");

    mega_ext->notify_chan = g_io_channel_unix_new(mega_ext->notify_sock);
    if (!mega_ext->notify_chan) {
        g_warning("g_io_channel_unix_new() failed");
        mega_notify_client_destroy(mega_ext);
        return FALSE;
    }

    g_io_channel_set_line_term(mega_ext->notify_chan, "\n", -1);
    g_io_channel_set_close_on_unref(mega_ext->notify_chan, TRUE);

    if (!g_io_add_watch(mega_ext->notify_chan, G_IO_IN | G_IO_HUP, mega_notify_client_read, mega_ext)) {
        g_warning("g_io_add_watch() failed!");
        mega_notify_client_destroy(mega_ext);
        return FALSE;
    }

    return TRUE;
}

void mega_notify_client_destroy(MEGAExt *mega_ext)
{
    if (mega_ext->notify_chan) {
        g_io_channel_shutdown(mega_ext->notify_chan, FALSE, NULL);
        g_io_channel_unref(mega_ext->notify_chan);
        mega_ext->notify_chan = NULL;
    }
    if (mega_ext->notify_sock > 0)
        close(mega_ext->notify_sock);
    mega_ext->notify_sock = -1;
    mega_ext->syncs_received = FALSE;
}

static gboolean mega_notify_client_read(GIOChannel *notify_chan, GIOCondition condition, gpointer data)
{
    gchar *in_line, *p;
    gchar type;
    gsize term_pos;
    gsize length;
    GError *error = NULL;
    GIOStatus status;
    MEGAExt *mega_ext = (MEGAExt *)data;

    if (condition & G_IO_HUP) {
        g_warning("Failed to read data!");
        mega_notify_client_destroy(mega_ext);
        // start connection timer
        mega_notify_client_timer_start(mega_ext);
        return FALSE;
    }

    status = g_io_channel_read_line(notify_chan, &in_line, &length, &term_pos, &error);
    if (status != G_IO_STATUS_NORMAL || error) {
        g_warning("Failed to read data!");
        mega_notify_client_destroy(mega_ext);
        // start connection timer
        mega_notify_client_timer_start(mega_ext);
        return FALSE;
    }

    // type + newline at least
    if (length < 3) {
        g_warning("Failed to read data!");
        g_free(in_line);
        mega_notify_client_destroy(mega_ext);
        // start connection timer
        mega_notify_client_timer_start(mega_ext);
        return FALSE;
    }
    p = in_line;

    if (term_pos)
        p[term_pos] = '\0';

    type = p[0];
    p++;

    switch(type) {
        case 'P': // item state changed
            mega_ext_on_item_changed(mega_ext, p);
            break;
        case 'A': // sync folder added
            mega_ext_on_sync_add(mega_ext, p);
            mega_ext->syncs_received = TRUE;
            break;
        case 'D': // sync folder deleted
            mega_ext_on_sync_del(mega_ext, p);
            break;
        default:
            g_warning("Failed to read data!");
            g_free(in_line);
            mega_notify_client_destroy(mega_ext);
            // start connection timer
            mega_notify_client_timer_start(mega_ext);
            return FALSE;
    }

    g_free(in_line);

    return TRUE;
}
