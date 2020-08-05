#! /usr/bin/env python
# 	 SOPG 2018. TP 2.
#    Copyright (C) 2018  Ernesto Gigliotti.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
import json

print("Content-Type: text/html")
print("")

fp = open("/tmp/out0.txt", 'r')
out0State = fp.read()
fp.close()

fp = open("/tmp/out1.txt", 'r')
out1State = fp.read()
fp.close()

fp = open("/tmp/out2.txt", 'r')
out2State = fp.read()
fp.close()

fp = open("/tmp/out3.txt", 'r')
out3State = fp.read()
fp.close()

dic = {"msg":"El servicio se encuentra funcionando con normalidad","lines":[out0State[0],out1State[0],out2State[0],out3State[0],"0","0"]}

print(json.dumps(dic))

