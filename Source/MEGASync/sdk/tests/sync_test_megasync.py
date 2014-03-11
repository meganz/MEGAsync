"""
 Application for testing syncing algorithm

 (c) 2013-2014 by Mega Limited, Wellsford, New Zealand

 This file is part of the MEGA SDK - Client Access Engine.

 Applications using the MEGA API must present a valid application key
 and comply with the the rules set forth in the Terms of Service.

 The MEGA SDK is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 @copyright Simplified (2-clause) BSD License.

 You should have received a copy of the license along with this
 program.
"""

import sys
import os
import time
import subprocess
import platform
import logging
import sync_test_app

class SyncTestMegaSyncApp (sync_test_app.SyncTestApp):
    def __init__(self, work_dir, remote_folder):
        """
        work_dir: a temporary folder to place generated files
        remote_folder: a remote folder to sync
        """
        self.megasync_ch_in = None
        self.megasync_ch_out = None

        self.local_mount_in = os.path.join(work_dir, "sync_in")
        self.local_mount_out = os.path.join(work_dir, "sync_out")

        self.work_dir = os.path.join(work_dir, "tmp")
        self.remote_folder = remote_folder

        # init base class
        sync_test_app.SyncTestApp.__init__ (self, self.local_mount_in, self.local_mount_out, self.work_dir)

        try:
            os.makedirs (self.local_mount_in);
        except Exception, e:
            self.logger.error("Failed to create directory: %s", self.local_mount_in)
            exit (1)

        try:
            os.makedirs (self.local_mount_out);
        except Exception, e:
            self.logger.error("Failed to create directory: %s", self.local_mount_out)
            exit (1)


    def start_megasync (self, local_folder):
        """
        fork and launch "megasync" application
        local_folder: local folder to sync
        """
        # launch megasync
        base_path = os.path.join(os.path.dirname(__file__), '..')

        #XXX: currently on Windows megasync.exe is located in the sources root dir.
        if platform.system() == 'Windows':
            bin_path = os.path.join(base_path, "")
        else:
            bin_path = os.path.join(base_path, "examples")

        args = [os.path.join(bin_path, "megasync"), local_folder, self.remote_folder]
        try:
            ch = subprocess.Popen (args, shell = False)
        except OSError, e:
            self.logger.error( "Failed to start megasync")
            return None
        return ch

    def sync (self):
        time.sleep (5)

    def start (self):
        """
        prepare and run tests
        """
        self.logger.debug ("Launching megasync instances ..")
        # start "in" instance
        self.megasync_ch_in = self.start_megasync (self.local_mount_in)
        # start "out" instance
        self.megasync_ch_out = self.start_megasync (self.local_mount_out)
        # check both instances
        if self.megasync_ch_in == None or self.megasync_ch_out == None:
            self.logger.error("Failed to start megasync instance.")
            return False

        return True

    def finish(self, res):
        """
        kill megasync instances, remove temp folders
        """
        # kill instances
        try:
            self.megasync_ch_in.terminate ()
        except Exception, e:
            self.logger.error ("Failed to kill megasync processes !")

        try:
            self.megasync_ch_out.terminate ()
        except Exception, e:
            self.logger.error ("Failed to kill megasync processes !")


if __name__ == "__main__":
    if len (sys.argv) < 3:
        print "Please run as:  python " + sys.argv[0] + " [work dir] [remote folder name]"
        sys.exit (1)

    app = SyncTestMegaSyncApp (sys.argv[1], sys.argv[2])
    app.run ()
