#!/usr/bin/sh

if [ -e "$PWD/scripts/common-env.sh" ]; then
    TOP_DIR=$PWD
elif [ -e "$PWD/common-env.sh" ] || [ -d "../kernel" ]; then
    TOP_DIR=`dirname $PWD`
else
    echo "Please run this script in top directory or any directory one level deeper than top"
    exit 1
fi

DIFFS="$TOP_DIR/diffs"
DIFFS_PROJECTS=$DIFFS/projects
DIFFS_TOOLS=$DIFFS/tools
KERNEL="kernel"
PROJECTS="projects"
TOOLS="tools"

BACKUP_DIR="$TOP_DIR/backup"

#
# Patch kernel/
#
echo $TOP_DIR/$KERNEL
if [ ! -e $TOP_DIR/$KERNEL ]; then
    cp -r $BACKUP_DIR/$KERNEL $TOP_DIR
fi
cd $TOP_DIR/$KERNEL
rm -rf .git .github
patch -p 1 < $DIFFS/kernel.patch

#
# Patch projects/
#

mkdir -p $TOP_DIR/$PROJECTS
for proj in `ls $BACKUP_DIR/$PROJECTS`
do
    if [ ! -e $TOP_DIR/$PROJECTS/$proj ]; then
        cp -r $BACKUP_DIR/$PROJECTS/$proj $TOP_DIR/$PROJECTS
    fi

    cd $TOP_DIR/$PROJECTS/$proj
    rm -rf .git .github
    patch -p 1 < $DIFFS_PROJECTS/$proj.patch
done

#
# Patch tools/
#

mkdir -p $TOP_DIR/$TOOLS
for tool in `ls $BACKUP_DIR/$TOOLS`
do
    if [ ! -e $TOP_DIR/$TOOLS/$tool ]; then
        cp -r $BACKUP_DIR/$TOOLS/$tool $TOP_DIR/$TOOLS
    fi

    cd $TOP_DIR/$TOOLS/$tool
    rm -rf .git .github
    patch -p 1 < $DIFFS_TOOLS/$tool.patch
done
