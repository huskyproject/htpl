# $Id$
#
# Common makefiles part (include-file) for libhtpl

# Use defines before including this file:
# _OBJ	object files suffix begins with point (".o", ".obj", ... )
# _LIB	library files suffix begins with point (".a", ".lib", ... )
# _DLL	dinamic linking library files suffix begins with point (".dll", ".so", ...)
# LIBPREFIX  library filename prefix ("lib", "", ...)
# LIBSUFFIX  library filename suffix (compiler ID, three characters, somewhere empty)
# DLLPREFIX  dinamic linking library prefix (usually empty)
# DLLSUFFIX  dinamic linking library suffix (compiler ID or empty)
# _SRC_DIR  sources dir

# Version
VERMAJOR= 1
VERMINOR= 9
VERPATCH= 0
VERH	= $(VERMAJOR).$(VERMINOR)
VER	= $(VERH).$(VERPATCH)

# File name
LIBNAME = htpl
TARGETLIB = $(LIBPREFIX)$(LIBNAME)$(LIBSUFFIX)$(_LIB)
TARGETDLL = $(DLLPREFIX)$(LIBNAME)$(DLLSUFFIX)$(_DLL)

# Object files: programs
OBJS    = parse$(_OBJ) strutil$(_OBJ) token$(_OBJ) sections$(_OBJ) varlist$(_OBJ) format$(_OBJ)

# Header files
HEADERS	= format.h htpl.h sections.h parse.h strutil.h token.h varlist.h

# Sources directory
_SRC_DIR= ../src/

# Headers directory
H_DIR	= ../htpl/
