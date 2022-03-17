#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd "/Users/sam/Desktop/Cal Poly/cpe471/FinalProject/SourceCode/build"
  make -f /Users/sam/Desktop/Cal\ Poly/cpe471/FinalProject/SourceCode/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd "/Users/sam/Desktop/Cal Poly/cpe471/FinalProject/SourceCode/build"
  make -f /Users/sam/Desktop/Cal\ Poly/cpe471/FinalProject/SourceCode/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd "/Users/sam/Desktop/Cal Poly/cpe471/FinalProject/SourceCode/build"
  make -f /Users/sam/Desktop/Cal\ Poly/cpe471/FinalProject/SourceCode/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd "/Users/sam/Desktop/Cal Poly/cpe471/FinalProject/SourceCode/build"
  make -f /Users/sam/Desktop/Cal\ Poly/cpe471/FinalProject/SourceCode/build/CMakeScripts/ReRunCMake.make
fi

