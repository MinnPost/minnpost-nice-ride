#!/usr/bin/perl

while(<STDIN>)
  {
   if(m%</way>%)
     {
      print "    <!-- Special case for riding -->\n";
      print "\n";
      print "    <if>\n";
      print "      <output k=\"foot\"       v=\"no\"/>\n";
      print "      <output k=\"wheelchair\" v=\"no\"/>\n";
      print "      <output k=\"moped\"      v=\"no\"/>\n";
      print "      <output k=\"motorbike\"  v=\"no\"/>\n";
      print "      <output k=\"motorcar\"   v=\"no\"/>\n";
      print "      <output k=\"goods\"      v=\"no\"/>\n";
      print "      <output k=\"hgv\"        v=\"no\"/>\n";
      print "      <output k=\"psv\"        v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"bridge\" v=\"no\"/>\n";
      print "      <output k=\"tunnel\" v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"footroute\" v=\"no\"/>\n";
      print "    </if>\n";
      print "\n";
     }

   if(m%</relation>%)
     {
      print "    <!-- Special case for riding -->\n";
      print "\n";
      print "    <if>\n";
      print "      <output k=\"footroute\" v=\"no\"/>\n";
      print "    </if>\n";
      print "\n";
     }

   print;
  }
