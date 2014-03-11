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
import sync_test_app

class SyncTestMegaCliApp (sync_test_app.SyncTestApp):
    def __init__(self, local_mount_in, local_mount_out):
        """
        local_mount_in: local upsync folder
        local_mount_out: local downsync folder
        """
        self.work_dir = os.path.join(".", "work_dir")
        sync_test_app.SyncTestApp.__init__ (self, local_mount_in, local_mount_out, self.work_dir)

    def sync (self):
        time.sleep (5)

    def start (self):
        # try to create work dir
        return True

    def finish (self, res):
        if res:
            # remove work dir
            try:
                shutil.rmtree (self.work_dir)
            except:
                None
        pass

if __name__ == "__main__":
    if len (sys.argv) < 3:
        print "Please run as:  python " + sys.argv[0] + " [upsync folder] [downsync folder]"
        sys.exit (1)

    print ""
    print "1) Start the first [megacli] and run:  sync " + sys.argv[1] + " [remote folder]"
    print "2) Start the second [megacli] and run:  sync " + sys.argv[2] + " [remote folder]"
    print "3) Wait for both folders get fully synced"
    print "4) Run sync_test.py"
    print ""
    app = SyncTestMegaCliApp (sys.argv[1], sys.argv[2])
    app.run ()
