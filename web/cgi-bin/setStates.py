#! /usr/bin/env python
# 	 PHP Conference 2014. Demo Desarrollo con Python sobre Sistemas Embebidos. 
#    Copyright (C) 2014  Ernesto Gigliotti. Laboratorio de Software Libre UTN-FRA
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
print("Content-Type: text/html")
print("")
import cgi


arguments = cgi.FieldStorage()

try:
	l0 = arguments["l0"].value
	l1 = arguments["l1"].value
	l2 = arguments["l2"].value
	l3 = arguments["l3"].value


	fp = open("/tmp/out0.txt", 'w+')
	fp.write(l0)
	fp.close()

	fp = open("/tmp/out1.txt", 'w+')
	fp.write(l1)
	fp.close()

	fp = open("/tmp/out2.txt", 'w+')
	fp.write(l2)
	fp.close()

	fp = open("/tmp/out3.txt", 'w+')
	fp.write(l3)
	fp.close()

	print("OK")

except Exception as e:
	print(e)
