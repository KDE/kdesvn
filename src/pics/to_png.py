#!/usr/bin/python
# -*- coding: utf-8 -*-

import os

def convert(_file, px):
    new_file=_file.replace('.svgz','.png').replace('.svg','.png').replace('hisc-','')
    os.system('/usr/bin/inkscape -e hicolor/hi%s-%s -w %s -h %s  %s' % (px, new_file,  px, px,  _file))

all_files=[
'hisc-action-kdesvnaddrecursive.svgz',
'hisc-action-kdesvnadd.svgz',
'hisc-action-kdesvnblame.svgz',
'hisc-action-kdesvncat.svgz',
'hisc-action-kdesvncheckout.svgz',
'hisc-action-kdesvncheckupdates.svgz',
'hisc-action-kdesvncleanup.svgz',
'hisc-action-kdesvncommit.svgz',
'hisc-action-kdesvncopy.svgz',
'hisc-action-kdesvndelete.svgz',
'hisc-action-kdesvndiff.svgz',
'hisc-action-kdesvnexport.svgz',
'hisc-action-kdesvninfo.svgz',
'hisc-action-kdesvnlock.svgz',
'hisc-action-kdesvnlog.svgz',
'hisc-action-kdesvnmerge.svgz',
'hisc-action-kdesvnrelocate.svgz',
'hisc-action-kdesvnswitch.svgz',
'hisc-action-kdesvnunlock.svgz',
'hisc-action-kdesvnupdate.svgz',
'hisc-action-kdesvnleft.svgz',
'hisc-action-kdesvnright.svgz',
'hisc-filesys-kdesvnadded.svgz',
'hisc-filesys-kdesvnconflicted.svgz',
'hisc-filesys-kdesvndeleted.svgz',
'hisc-filesys-kdesvnlocked.svgz',
'hisc-filesys-kdesvnmodified.svgz',
'hisc-filesys-kdesvnupdates.svgz',
'hisc-filesys-kdesvnneedlock.svgz',
'hisc-action-kdesvnrightreload.svgz',
'hisc-action-kdesvnreverse.svgz',
'hisc-action-kdesvntree.svgz',
'hisc-action-kdesvnresolved.svgz',
'hisc-action-kdesvnclock.svgz'
]

i_sizes=[
16,22,32,48,64,96,128]

i_sizes.sort()

os.system('mkdir -p hicolor')


#for px in i_sizes:
#    os.system('mkdir -p hicolor/%sx%s/actions' % (px,px))
#    os.system('mkdir -p hicolor/%sx%s/filesystems' % (px,px))

for _file in all_files:
    for px in i_sizes:
        convert(_file, px)
