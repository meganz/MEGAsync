"""
 Application for stress testing syncing algorithm

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
from threading import Thread
import random
import time
import string
import shutil
import logging
import collections

class Action:
    dir_type = "dir"
    file_type = "file"

    def __init__ (self, base_sync_dir, tmp_dir, otype):
        self.base_sync_dir = base_sync_dir
        self.otype = otype
        self.tmp_dir = tmp_dir
        self.seed = "MegaSDK"
        self.words = open("lorem.txt", "r").read().replace("\n", '').split()
        self.max_files = 100
        self.full_name = ""
        self.base_name = ""

    def is_dir (self):
        return self.otype == self.dir_type

    def is_file (self):
        return self.otype == self.file_type

    def get_random_str (self, size=10, chars = string.ascii_uppercase + string.digits):
        return ''.join (random.choice (chars) for x in range (size))

    def fdata (self):
        """
        Produce random data
        """
        a = collections.deque (self.words)
        b = collections.deque (self.seed)
        while True:
            yield ' '.join (list (a)[0:1024])
            a.rotate (int (b[0]))
            b.rotate (1)


    def create_file (self, fname, flen):
        """
        Create a file with the given length
        """
        g = self.fdata ()
        fout = open (fname, 'w')
        while os.path.getsize (fname) < flen:
            fout.write (g.next())
        fout.close ()

    def create (self):
        """
        Create a file / dir with a random name
        Must be the first action to be called
        """
        # generate a random file / dir name
        self.base_name = self.get_random_str ()
        # set a full name
        self.full_name = self.base_sync_dir + "/" + self.base_name

        if self.is_dir ():
            try:
                os.mkdir (self.full_name);
            except Exception, e:
                print "Failed to create dir !", e
                sys.exit (1)
        else:
            try:
                f = open (self.full_name, 'w')
            except Exception, e:
                print "Failed to create file !", e
                sys.exit (1)
            f.close ()
        logging.info ('[create] [%s] %s', self.otype, self.full_name)


    def rename (self):
        """
        Rename a file / dir
        """
        self.base_name = self.get_random_str ()
        new_name = self.base_sync_dir + "/" + self.base_name

        try:
            shutil.move (self.full_name, new_name)
        except Exception, e:
            print "Failed to move an object !", e
            sys.exit (1)
        logging.info ('[rename] [%s] %s => %s', self.otype, self.full_name, new_name)
        self.full_name = new_name

    def moveout (self):
        """
        Move out of sync dir
        """
        new_name = self.tmp_dir + "/" + self.base_name

        try:
            shutil.move (self.full_name, new_name)
        except Exception, e:
            print "Failed to move an object !", e
            sys.exit (1)

        logging.info ('[moveout] [%s] %s => %s', self.otype, self.full_name, new_name)
        self.full_name = new_name


    def movein (self):
        """
        Move into sync dir
        """
        new_name = self.base_sync_dir + "/" + self.base_name

        try:
            shutil.move (self.full_name, new_name)
        except Exception, e:
            print "Failed to move an object !", e
            sys.exit (1)

        logging.info ('[movein] [%s] %s => %s', self.otype, self.full_name, new_name)
        self.full_name = new_name

    def delete (self):
        """
        Delete file / dir
        """
        if self.is_dir ():
            try:
                shutil.rmtree (self.full_name)
            except Exception, e:
                print "Failed to delete directory !", e
                sys.exit (1)
        else:
            try:
                os.unlink (self.full_name)
            except Exception, e:
                print "Failed to delete file !", e
                sys.exit (1)
        logging.info ('[delete] [%s] %s', self.otype, self.full_name)
        self.full_name = ""
        self.base_name = ""

    def filldir (self):
        """
        fill dir with random files
        """
        if self.is_file ():
            return

        max_files = random.randint (1, self.max_files)
        for i in range (0, max_files):
            flen = random.randint (1, 1024 * 4)
            fname = self.get_random_str ()
            self.create_file (self.full_name + "/" + fname, flen)

        logging.info ('[filldir] [%s] created %d files', self.otype, max_files)


class Worker (Thread):
    """
    Worker class (run in a thread)
    """
    def __init__ (self, base_sync_dir, tmp_dir):
        Thread.__init__(self)
        self.base_sync_dir = base_sync_dir
        self.tmp_dir = tmp_dir

    def cmds (self):
        f = open ("cmds.txt", "r")
        l_cmds = []
        for line in f:
            if (line[0] != '#') and (len (line) > 1):
                l_cmds.append (line.strip())
        f.close()
        i = 0
        while i < len (l_cmds):
            yield l_cmds[i]
            i = i + 1

    def run (self):
        otype = [Action.dir_type, Action.file_type]
        a = Action (self.base_sync_dir, self.tmp_dir, random.choice (otype))
        cmd = self.cmds ()
        for c in cmd:
            if c == "create":
                a.create ()
            elif c == "moveout":
                a.moveout ()
            elif c == "movein":
                a.movein ()
            elif c == "rename":
                a.rename ()
            elif c == "filldir":
                a.filldir ()
            elif c == "delete":
                a.delete ()
            elif c == "sleep":
                logging.info ('[sleep]')
                time.sleep (2)
            else:
                logging.error ('Unknown command: %s', c)


def main (num_workers, base_sync_dir, tmp_dir):
    random.seed (time.time())

    w_list = []
    for i in range (0, num_workers):
        w = Worker (base_sync_dir, tmp_dir)
        w_list.append (w)

    for w in w_list:
        w.run ()

if __name__ == "__main__":
    if len (sys.argv) < 4:
        print "Please run as: " + sys.argv[0] + " [number of workers] [sync directory] [out of sync directory]"
        sys.exit (1)

    # logging stuff, output to stdout
    logging.StreamHandler (sys.stdout)
    logging.basicConfig (level=logging.INFO)

    main (int (sys.argv[1]), sys.argv[2], sys.argv[3])
