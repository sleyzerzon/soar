###
# Copyright 1995-2004 Soar Technology, Inc., University of Michigan. All 
# rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are 
# met:
# 
#    1.	Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer. 
# 
#    2.	Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SOAR 
# CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
# DAMAGE.
# 
# The views and conclusions contained in the software and documentation 
# are those of the authors and should not be interpreted as representing 
# official policies, either expressed or implied, of Soar Technology, Inc., 
# the University of Michigan, or the Soar consortium.
### 

##
# Code for @file documentation blocks 
import DocBlock
import string

class FileBlock(DocBlock.DocBlock):
   def __init__(self, sourceFileName, lineNo, isTemp, block):
      DocBlock.DocBlock.__init__(self, sourceFileName, lineNo, isTemp)
      self.Prods = [] # list of production names...
      self.__projects = []
      self.ProblemSpaces = []
      self.Operators = []
      self.ProductionType = None
      for cmd, args in block:
         if self.handleBlockCommand(cmd, args):
            continue
         elif cmd == 'project':
            self.__projects.extend(map(string.strip, args.split()))
         elif cmd == 'problem-space':
            self.ProblemSpaces.extend(map(string.strip, args.split()))
         elif cmd == 'operator':
            self.Operators.extend(map(string.strip, args.split()))
         elif cmd == 'production-type' or cmd == 'type':
            if len(args) > 0:
               self.ProductionType = args.split()[0].strip()
         else:
            pass
      if self.GetName() == '':
         self.SetName(sourceFileName)

   def GetProjects(self): return self.__projects
   
   def Accept(self, visitor):
      return visitor.VisitFileBlock(self)
      
   def GetNameCommand(self): return 'file'

##   def __str__(self):
##      return '%s\nProjects=%s\nCreated=%s\nMods=%s' % (DocBlock.DocBlock.__str__(self),
##                                          self.GetProjects(),
##                                          self.GetCreated(),
##                                          self.GetMods())
