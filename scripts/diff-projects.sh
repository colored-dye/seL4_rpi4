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

mkdir -p $DIFFS
mkdir -p $DIFFS/projects
mkdir -p $DIFFS/tools

#
# Diff kernel/
#

KERNEL_DIR="kernel"
BACKUP_KERNEL_DIR="backup/kernel"

echo "$TOP_DIR/kernel"
cd $TOP_DIR
diff -Nur --exclude=.git --exclude=.github --exclude-from=$KERNEL_DIR/.gitignore $BACKUP_KERNEL_DIR $KERNEL_DIR > $DIFFS/kernel.patch

#
# Diff projects/
#

PROJECTS_DIR="$TOP_DIR/projects"
BACKUP_PROJECTS_DIR="../backup/projects"

cd $PROJECTS_DIR
for proj in `ls $PROJECTS_DIR`
do
    echo $PROJECTS_DIR/$proj
    
    diff -Nur --exclude=.git --exclude=.github --exclude=*.pyc $BACKUP_PROJECTS_DIR/$proj $proj > $DIFFS/projects/$proj.patch
done

#
# Diff tools/
#

TOOLS_DIR="$TOP_DIR/tools"
BACKUP_TOOLS_DIR="../backup/tools"
cd $TOOLS_DIR
for tool in `ls $TOOLS_DIR`
do
    echo $TOOLS_DIR/$tool

    diff -Nur --exclude=.git --exclude=.github --exclude=*.pyc --exclude-from=$tool/.gitignore $BACKUP_TOOLS_DIR/$tool $tool > $DIFFS/tools/$tool.patch
done
