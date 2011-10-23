#!/usr/bin/python
import commands, re

cpp=commands.getoutput("find . -name '*.cpp' | xargs wc -l |grep celkem")
headers=commands.getoutput("find . -name '*.h' | xargs wc -l |grep celkem")

intcpp=int(re.findall(r"\d+",cpp)[0])
intheaders=int(re.findall(r"\d+",headers)[0])

print "\n"
print "##########################"
print "## nowrep line counter! ##"
print "##########################"
print "\n"
print "Lines in Headers (.h files): "+str(intheaders)
print "Lines in Sources (.cpp files): "+str(intcpp)
print "----------------------------------------"
print "\n"
print ":::  "+str(intheaders + intcpp)+" LINES IN SUMMARY  :::"
print "\n"
