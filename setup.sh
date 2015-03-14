# add directory to a path variable
add_to_envVar () {
  ADDPATH=$1
  VAR=$2
  ERR=0
  if [ -z $ADDPATH ] || [ ! -d $ADDPATH  ] ; then
    echo "Path undefined"
    return 1
  fi
  if [ -z $VAR ] ; then
    echo "Variable undefined"
    return 1
  fi

  eval PVAR=\$$VAR
  if [ -z $PVAR ] ; then
      export ${VAR}=$ADDPATH
      echo creating $PVAR \$$VAR
      return 0
  fi
  
  path_list=`echo $PVAR | tr ':' ' '`
  for d in $path_list ; do
      if [ $d == $ADDPATH ] ; then
	  return 0
      fi
  done
  export ${VAR}=${ADDPATH}:${PVAR}
}

export PULSEGUI_DIR=$PWD
export PULSEGUI_INC=${PULSEGUI_DIR}/include
export PULSEGUI_LIBDIR=${PULSEGUI_DIR}/build/lib



add_to_envVar ${PULSEGUI_DIR}/bin       PATH
add_to_envVar ${PULSEGUI_DIR}/python    PATH
add_to_envVar ${PULSEGUI_DIR}/python   PYTHONPATH
add_to_envVar ${PULSEGUI_DIR}/build/lib  LD_LIBRARY_PATH


# hack until someone figures out how to check for root-config in the Makefile
if [ -e /usr/bin/root ] ; then
  if [ -z $ROOTSYS ] ; then export ROOTSYS="." ; fi
fi

