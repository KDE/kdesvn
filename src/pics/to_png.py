#!/usr/bin/python

import os

def convert(file, px):
    new_file=file.replace('.svgz','.png').replace('.svg','.png').replace('hisc-','').replace('action-','actions/').replace('filesys-','filesystems/')
    os.system('/usr/bin/inkscape -e hicolor/%sx%s/%s -w %s -h %s  %s' % (px,px, new_file,  px, px,  file))
    
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
'hisc-filesys-kdesvnadded.svgz',
'hisc-filesys-kdesvnconflicted.svgz',
'hisc-filesys-kdesvndeleted.svgz',
'hisc-filesys-kdesvnlocked.svgz',
'hisc-filesys-kdesvnmodified.svgz',
'hisc-filesys-kdesvnupdates.svgz',
]

i_sizes=[
32,128,16,96,22,48,64]

i_sizes.sort()

for px in i_sizes:
    os.system('mkdir -p hicolor/%sx%s/actions' % (px,px))
    os.system('mkdir -p hicolor/%sx%s/filesystems' % (px,px))

for file in all_files:
    for px in i_sizes:
	convert(file, px)
