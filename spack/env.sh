export SAP_HOME=/usr/projects/silverton

if [ ! -d "$SAP_HOME" ]; then
   echo "SAP_HOME does not exist: ${SAP_HOME}"
   return 1
fi

export SAP_DEVENV_MODULES=${SAP_HOME}/dev_tools/env/spack/share/spack/modules/env
if [ ! -d "$SAP_DEVENV_MODULES" ]; then
   echo "SAP_DEVENV_MODULES does not exist: ${SAP_DEVENV_MODULES}"
   return 1
fi

module use $SAP_DEVENV_MODULES
module use ${SAP_DEVENV_MODULES}_v4
module load llvm neovim py-fortls fd ripgrep
