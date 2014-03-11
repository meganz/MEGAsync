"""
 Base class for testing syncing algorithm

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
import signal
import hashlib
import subprocess

class SyncTestApp ():
    def __init__(self, local_mount_in, local_mount_out, work_folder):
        """
        local_mount_in: local upsync folder
        local_mount_out: local downsync folder
        work_folder: a temporary out of sync folder
        """
        random.seed (time.time())

        self.local_mount_in = local_mount_in
        self.local_mount_out = local_mount_out

        self.rnd_folder = self.get_random_str ()
        self.local_folder_in = os.path.join (self.local_mount_in, self.rnd_folder)
        self.local_folder_out = os.path.join (self.local_mount_out, self.rnd_folder)
        self.work_folder = os.path.join (work_folder, self.rnd_folder)

        self.nr_retries = 10
        self.nr_files = 5
        self.nr_dirs = 5
        self.l_files = []
        self.l_dirs = []

        self.seed = "0987654321"
        self.words = open("lorem.txt", "r").read().replace("\n", '').split()

        self.log_file_name = "sync_test.log"

        self.logger = logging.getLogger(__name__)

        self.ch = logging.StreamHandler (sys.stdout)
        formatter = logging.Formatter('[%(asctime)s] %(message)s', datefmt='%H:%M:%S')
        self.ch.setFormatter (formatter)
        self.ch.setLevel (logging.INFO)
        self.logger.addHandler (self.ch)

        fh = logging.FileHandler(self.log_file_name, mode='w')
        formatter = logging.Formatter('[%(asctime)s] %(message)s', datefmt='%H:%M:%S')
        fh.setFormatter (formatter)
        fh.setLevel (logging.DEBUG)
        self.logger.addHandler (fh)

        self.logger.setLevel (logging.DEBUG)

    def get_random_str (self, size=10, chars = string.ascii_uppercase + string.digits):
        """
        return a random string
        size: size of an output stirng
        chars: characters to use
        """
        return ''.join (random.choice (chars) for x in range (size))

    def fdata (self):
        """
        return random data
        """
        a = collections.deque (self.words)
        b = collections.deque (self.seed)
        while True:
            yield ' '.join (list (a)[0:1024])
            a.rotate (int (b[0]))
            b.rotate (1)

    def test_empty (self, folder_name):
        """
        return True if folder is empty
        """
        return not os.listdir(folder_name)

    def create_file (self, fname, flen):
        """
        create a file of a length flen and fill with a random data
        """
        g = self.fdata ()
        fout = open (fname, 'w')
        while os.path.getsize (fname) < flen:
            fout.write (g.next())
        fout.close ()

    def create_files (self):
        """
        create files in "in" instance and check files presence in "out" instance
        """
        # generate and put files into "in" folder

        # empty files
        for i in range (self.nr_files):
            strlen = random.randint (0, 20)
            fname = "e" + self.get_random_str (size=strlen) + str (i)
            ffname = os.path.join (self.local_folder_in, fname)
            flen = 0
            try:
                self.create_file (ffname, flen)
            except Exception, e:
                self.logger.error("Failed to create file: %s", ffname)
                return False
            md5_str = self.md5_for_file (ffname)
            self.l_files.append ({"name":fname, "len":flen, "md5":md5_str, "name_orig":fname})
            self.logger.debug ("File created: %s [%s, %db]", ffname, md5_str, flen)

        # small files < 1k
        for i in range (self.nr_files):
            strlen = random.randint (0, 20)
            fname = "s" + self.get_random_str (size=strlen) + str (i)
            ffname = os.path.join (self.local_folder_in, fname)
            flen = random.randint (1, 1*1024)
            try:
                self.create_file (ffname, flen)
            except Exception, e:
                self.logger.error("Failed to create file: %s", ffname)
                return False
            md5_str = self.md5_for_file (ffname)
            self.l_files.append ({"name":fname, "len":flen, "md5":md5_str, "name_orig":fname})
            self.logger.debug ("File created: %s [%s, %db]", ffname, md5_str, flen)

        # medium files < 1mb
        for i in range (self.nr_files):
            strlen = random.randint (0, 20)
            fname = "m" + self.get_random_str (size=strlen) + str (i)
            ffname = os.path.join (self.local_folder_in, fname)
            flen = random.randint (1, 1*1024*1024)
            try:
                self.create_file (ffname, flen)
            except Exception, e:
                self.logger.error("Failed to create file: %s", ffname)
                return False
            md5_str = self.md5_for_file (ffname)
            self.l_files.append ({"name":fname, "len":flen, "md5":md5_str, "name_orig":fname})
            self.logger.debug ("File created: %s [%s, %db]", ffname, md5_str, flen)

        # large files < 10mb
        for i in range (self.nr_files):
            strlen = random.randint (0, 20)
            fname = "l" + self.get_random_str (size=strlen) + str (i)
            ffname = os.path.join (self.local_folder_in, fname)
            flen = random.randint (1, 10*1024*1024)
            try:
                self.create_file (ffname, flen)
            except Exception, e:
                self.logger.error("Failed to create file: %s", ffname)
                return False
            md5_str = self.md5_for_file (ffname)
            self.l_files.append ({"name":fname, "len":flen, "md5":md5_str, "name_orig":fname})
            self.logger.debug ("File created: %s [%s, %db]", ffname, md5_str, flen)

        # randomize list
        random.shuffle (self.l_files)

        return True

    def md5_for_file (self, fname, block_size=2**20):
        """
        calculates md5 of a file
        """
        fout = open (fname, 'r')
        md5 = hashlib.md5()
        while True:
            data = fout.read(block_size)
            if not data:
                break
            md5.update(data)
        fout.close ()
        return md5.hexdigest()

    def check_files (self, l_files, dir_name=""):
        """
        check files on both folders
        compare names, len, md5 sums
        """
        # check files
        for f in l_files:
            dd_out = os.path.join (self.local_folder_out, dir_name)
            ffname = os.path.join (dd_out, f["name"])

            dd_in = os.path.join (self.local_folder_in, dir_name)
            ffname_in = os.path.join (dd_in, f["name"])

            success = False

            self.logger.debug ("Comparing %s and %s", ffname_in, ffname)
            # try to access the file
            for r in range (0, self.nr_retries):
                try:
                    with open(ffname) as fil: pass
                    success = True
                    break;
                except:
                    # wait for a file
                    self.logger.debug ("File %s not found! Retrying [%d/%d] ..", ffname, r + 1, self.nr_retries)
                    self.sync ()
            if success == False:
                self.logger.error("Failed to compare files: %s and %s", ffname_in, ffname)
                return False
            # get md5 of synced file
            md5_str = self.md5_for_file (ffname)
            if md5_str != f["md5"]:
                self.logger.error("MD5 sums don't match for file: %s", ffname)
                return False
        return True

    def create_dir (self, dname, files_num):
        """
        create and fill directory with files
        return files list
        """
        try:
            os.makedirs (dname);
        except Exception, e:
            self.logger.error("Failed to create directory: %s", dnamedname)
            return None

        l_files = []
        for i in range (0, files_num):
            strlen = random.randint (0, 20)
            fname = "d" + self.get_random_str (size=strlen) + str (i)
            ffname = os.path.join (dname, fname)
            flen = random.randint (1, 1*1024)
            try:
                self.create_file (ffname, flen)
            except Exception, e:
                self.logger.error("Failed to create file: %s", ffname)
                return None
            md5_str = self.md5_for_file (ffname)
            l_files.append ({"name":fname, "len":flen, "md5":md5_str, "name_orig":fname})
            self.logger.debug ("File created: %s [%s, %db]", ffname, md5_str, flen)

        return l_files

    def check_dirs (self):
        for d in self.l_dirs:
            dname = os.path.join (self.local_folder_out, d["name"])
            dname_in = os.path.join (self.local_folder_in, d["name"])
            success = False

            self.logger.debug ("Comparing dirs: %s and %s", dname_in, dname)

            # try to access the dir
            for r in range (0, self.nr_retries):
                try:
                    if os.path.isdir (dname):
                        success = True
                        break;
                    else:
                        # wait for a dir
                        self.logger.debug ("Directory %s not found! Retrying [%d/%d] ..", dname, r + 1, self.nr_retries)
                        self.sync ()
                except:
                    # wait for a dir
                    self.logger.debug ("Directory %s not found! Retrying [%d/%d] ..", dname, r + 1, self.nr_retries)
                    self.sync ()
            if success == False:
                self.logger.error("Failed to access directories: %s and ", dname)
                return False

            # check files
            res = self.check_files (d["l_files"], d["name"])
            if not res:
                self.logger.error("Directories do not match !")
                return False

        return True

    def create_dirs (self):
        """
        create dirs
        """
        # create empty dirs
        dname = "z"
        ddname = os.path.join (self.local_folder_in, dname)
        files_nr = 0
        try:
            l_files = self.create_dir (ddname, files_nr)
        except Exception, e:
            self.logger.error("Failed to create directory: %s", ddname)
            return False
        self.l_dirs.append ({"name":dname, "files_nr":files_nr, "name_orig":dname, "l_files":l_files})
        self.logger.debug ("Directory created: %s [%d files]", ddname, files_nr)

        for i in range (self.nr_dirs):
            strlen = random.randint (0, 20)
            dname = "z" + self.get_random_str (size=strlen) + str (i)
            ddname = os.path.join (self.local_folder_in, dname)
            files_nr = 0
            try:
                l_files = self.create_dir (ddname, files_nr)
            except Exception, e:
                self.logger.error("Failed to create directory: %s", ddname)
                return False
            self.l_dirs.append ({"name":dname, "files_nr":files_nr, "name_orig":dname, "l_files":l_files})
            self.logger.debug ("Directory created: %s [%d files]", ddname, files_nr)

        # create dirs with < 20 files
        dname = "d"
        ddname = os.path.join (self.local_folder_in, dname)
        files_nr = random.randint (1, 20)
        try:
            l_files = self.create_dir (ddname, files_nr)
        except Exception, e:
            self.logger.error("Failed to create directory: %s", ddname)
            return False
        self.l_dirs.append ({"name":dname, "files_nr":files_nr, "name_orig":dname, "l_files":l_files})
        self.logger.debug ("Directory created: %s [%d files]", ddname, files_nr)

        for i in range (self.nr_dirs):
            strlen = random.randint (0, 20)
            dname = "d" + self.get_random_str (size=strlen) + str (i)
            ddname = os.path.join (self.local_folder_in, dname)
            files_nr = random.randint (1, 20)
            try:
                l_files = self.create_dir (ddname, files_nr)
            except Exception, e:
                self.logger.error("Failed to create directory: %s", ddname)
                return False
            if l_files == None:
                self.logger.error("Failed to create directory: %s", ddname)
                return False
            self.l_dirs.append ({"name":dname, "files_nr":files_nr, "name_orig":dname, "l_files":l_files})
            self.logger.debug ("Directory created: %s [%d files]", ddname, files_nr)

        # randomize list
        random.shuffle (self.l_dirs)

        return True

    def files_rename (self):
        """
        rename objects in "in" instance and check new files in "out" instance
        """
        for f in self.l_files:
            ffname_src = os.path.join (self.local_folder_in, f["name"])
            f["name"] = "renamed_" + self.get_random_str (30)
            ffname_dst = os.path.join (self.local_folder_in, f["name"])
            try:
                os.rename (ffname_src, ffname_dst)
            except:
                self.logger.error("Failed to rename file: %s", ffname_src)
                return False
            self.logger.debug ("File renamed: %s => %s", ffname_src, ffname_dst)
        return True

    def files_remove (self):
        """
        remove files in "in" instance and check files absence in "out" instance
        """

        for f in self.l_files:
            ffname = os.path.join (self.local_folder_in, f["name"])
            try:
                os.remove (ffname)
            except:
                self.logger.error("Failed to delete file: %s", ffname)
                return False
            self.logger.debug ("File deleted: %s", ffname)

        # give some time to sync files to remote folder
        self.sync ()

        success = False
        for f in self.l_files:
            ffname = os.path.join (self.local_folder_out, f["name"])
            for r in range (0, self.nr_retries):
                try:
                    # file must be deleted
                    with open(ffname) as fil: pass
                    self.logger.debug ("File %s is not deleted. Retrying [%d/%d] ..", ffname, r + 1, self.nr_retries)
                    self.sync ()
                except:
                    success = True
                    break;
            if success == False:
                self.logger.error("Failed to delete file: %s", ffname)
                return False
        return True

    def dirs_remove (self):
        """
        remove directories in "in" instance and check directories absence in "out" instance
        """

        for d in self.l_dirs:
            dname = os.path.join (self.local_folder_in, d["name"])
            try:
                shutil.rmtree (dname)
            except:
                self.logger.error("Failed to delete file: %s", dname)
                return False
            self.logger.debug ("Directory removed: %s", dname)

        # give some time to sync files to remote folder
        self.sync ()

        success = False
        for f in self.l_dirs:
            dname = os.path.join (self.local_folder_out, d["name"])
            for r in range (0, self.nr_retries):
                try:
                    # dir must be deleted
                    if not os.path.isdir (dname):
                        success = True
                        break;
                    self.logger.debug ("Retrying [%d/%d] ..", r + 1, self.nr_retries)
                    self.sync ()
                except:
                    success = True
                    break;
            if success == False:
                self.logger.error("Failed to delete dir: %s", dname)
                return False
        return True

    def dirs_rename (self):
        """
        rename directories in "in" instance and check directories new names in "out" instance
        """
        for d in self.l_dirs:
            dname_src = os.path.join (self.local_folder_in, d["name"])
            d["name"] = "renamed_" + self.get_random_str (30)
            dname_dst = os.path.join (self.local_folder_in, d["name"])
            try:
                os.rename (dname_src, dname_dst)
            except:
                self.logger.error("Failed to rename directory: %s", dname_src)
                return False
            self.logger.debug ("Directory renamed: %s => %s", dname_src, dname_dst)

        return True

    def do_test (self, func, msg, *args):
        """
        execute test and print corresponding message
        """
        self.logger.info (msg + " ..")
        sys.stdout.flush()
        self.ch.flush ()
        if func (*args):
            self.logger.info (msg + " [SUCCESS]")
            return True
        else:
            self.logger.error (msg + " [Failed]")
            self.cleanup (False)
            return False

    def test_folders_empty (self):
        return self.test_empty (self.local_folder_in) and self.test_empty (self.local_folder_out)

    def test_create_delete_files (self):
        """
        create files with different size,
        compare files on both folders,
        remove files, check that files removed from the second folder
        """

        self.l_files = []

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        # create files
        if not self.do_test (self.create_files, "Creating files"):
            return False

        self.sync ()

        # comparing
        if not self.do_test (self.check_files, "Comparing files", self.l_files):
            return False

        # remove files
        if not self.do_test (self.files_remove, "Removing files"):
            return False

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        return True

    def test_create_rename_delete_files (self):
        """
        create files with different size,
        compare files on both folders,
        rename files
        compare files on both folders,
        remove files, check that files removed from the second folder
        """

        self.l_files = []

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        # create files
        if not self.do_test (self.create_files, "Creating files"):
            return False

        self.sync ()

        # comparing
        if not self.do_test (self.check_files, "Comparing files", self.l_files):
            return False

        # renaming
        if not self.do_test (self.files_rename, "Renaming files"):
            return False

        self.sync ()

        # comparing
        if not self.do_test (self.check_files, "Comparing files", self.l_files):
            return False

        # remove files
        if not self.do_test (self.files_remove, "Removing files"):
            return False

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        return True

    def test_create_delete_dirs (self):
        """
        create directories with different amount of files,
        compare directories on both sync folders,
        remove directories, check that directories removed from the second folder
        """

        self.l_dirs = []

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        # create dirs
        if not self.do_test (self.create_dirs, "Creating directories"):
            return False

        self.sync ()

        # comparing
        if not self.do_test (self.check_dirs, "Comparing directories"):
            return False

        # remove files
        if not self.do_test (self.dirs_remove, "Removing directories"):
            return False

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        return True

    def test_create_rename_delete_dirs (self):
        """
        create directories with different amount of files,
        compare directories on both sync folders,
        rename directories
        compare directories on both sync folders,
        remove directories, check that directories removed from the second folder
        """

        self.l_dirs = []

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        # create dirs
        if not self.do_test (self.create_dirs, "Creating directories"):
            return False

        self.sync ()

        # comparing
        if not self.do_test (self.check_dirs, "Comparing directories"):
            return False

        # rename dirs
        if not self.do_test (self.dirs_rename, "Rename directories"):
            return False

        self.sync ()

        # comparing
        if not self.do_test (self.check_dirs, "Comparing directories"):
            return False

        # remove files
        if not self.do_test (self.dirs_remove, "Removing directories"):
            return False

        # make sure remote folders are empty
        if not self.do_test (self.test_folders_empty, "Checking if remote folders are empty"):
            return False

        return True

# virtual functions
    def start (self):
        raise NotImplementedError("Not Implemented !")
    def finsh (self, res):
        raise NotImplementedError("Not Implemented !")
    def sync (self):
        """
        function waits while two folders are synchronized
        """
        raise NotImplementedError("Not Implemented !")

    def prepare_folders (self):
        """
        prepare upsync, downsync and work directories
        """
        # create "in" folder
        self.logger.info ("IN folder: %s", self.local_folder_in)
        try:
            os.makedirs (self.local_folder_in);
        except Exception, e:
            self.logger.error("Failed to create directory: %s", self.local_folder_in)
            return False

        self.logger.info ("OUT folder: %s", self.local_folder_out)
        success = False
        # try to access the dir
        for r in range (0, self.nr_retries):
            try:
                if os.path.isdir (self.local_folder_out):
                    success = True
                    break;
                else:
                    # wait for a dir
                    self.logger.debug ("Directory %s not found! Retrying [%d/%d] ..", self.local_folder_out, r + 1, self.nr_retries)
                    self.sync ()
            except:
                # wait for a dir
                self.logger.debug ("Directory %s not found! Retrying [%d/%d] ..", self.local_folder_out, r + 1, self.nr_retries)
                self.sync ()
        if success == False:
            self.logger.error("Failed to access directory: %s", self.local_folder_out)
            return False

        # create work folder
        self.logger.info ("Work folder: %s", self.work_folder)
        try:
            os.makedirs (self.work_folder);
        except Exception, e:
            self.logger.error("Failed to create directory: %s", self.work_folder)
            return False

        self.logger.info ("Log file: %s", self.log_file_name)
        return True

    def run (self):
        """
        prepare and run tests
        """
        self.logger.info ("Starting ..")

        # call subclass function
        res = self.start ()
        if not res:
            self.logger.info ("Test: [FAILED]")
            self.cleanup (False)
            return

        res = self.prepare_folders ()
        if not res:
            self.logger.info ("Test: [FAILED]")
            self.cleanup (False)
            return

        self.sync ()

        #
        # run tests
        #
        self.logger.info ("Launching create / delete files test.")
        res = self.test_create_delete_files ()
        if not res:
            self.logger.info ("Test: [FAILED]")
            self.cleanup (False)
            return

        self.sync ()

        self.logger.info ("Launching create / rename / delete files test.")
        res = self.test_create_rename_delete_files ()
        if not res:
            self.logger.info ("Test: [FAILED]")
            self.cleanup (False)
            return

        self.sync ()

        self.logger.info ("Launching create / delete directories test.")
        res = self.test_create_delete_dirs ()
        if not res:
            self.logger.info ("Test: [FAILED]")
            self.cleanup (False)
            return

        self.sync ()

        self.logger.info ("Launching create / rename / delete directories test.")
        res = self.test_create_rename_delete_dirs ()
        if not res:
            self.logger.info ("Test: [FAILED]")
            self.cleanup (False)
            return

        #
        # tests done
        #

        self.cleanup (True)

    def cleanup(self, res):
        """
        """
        self.finish (res)

        # remove tmp folders if no errors
        if res == True:
            try:
                shutil.rmtree (self.local_folder_in)
            except:
                None
            try:
                shutil.rmtree (self.local_folder_out)
            except:
                None
            try:
                shutil.rmtree (self.work_folder)
            except:
                None

            self.logger.info ("SUCCESS !")
            sys.exit (0)
        else:
            self.logger.info ("Aborted, please check %s log file for errors!", self.log_file_name)
            sys.exit (1)
