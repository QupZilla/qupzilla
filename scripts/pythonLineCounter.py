#!/usr/bin/python
import commands, re

cpp=commands.getoutput("find . -name '*.cpp' | xargs wc -l |grep celkem")
headers=commands.getoutput("find . -name '*.h' | xargs wc -l |grep celkem")
html=commands.getoutput("find . -name '*.html' | xargs wc -l |grep celkem")

intcpp=int(re.findall(r"\d+",cpp)[0])
intheaders=int(re.findall(r"\d+",headers)[0])
inthtml=int(re.findall(r"\d+",html)[0])

print "##########################"
print "##       QupZilla       ##"
print "##########################"
print "\n"
print "Lines in Headers (.h files): "+str(intheaders)
print "Lines in Sources (.cpp files): "+str(intcpp)
print "Lines in Pages (.html files): "+str(inthtml)
print "----------------------------------------"
print "\n"
print ":::  "+str(intheaders + intcpp + inthtml)+" LINES IN SUMMARY  :::"
print "\n"
