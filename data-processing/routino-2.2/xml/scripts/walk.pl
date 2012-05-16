#!/usr/bin/perl

while(<STDIN>)
  {
   if(m%</way>%)
     {
      print "    <!-- Special case for walking -->\n";
      print "\n";
      print "    <if>\n";
      print "      <output k=\"horse\"      v=\"no\"/>\n";
      print "      <output k=\"bicycle\"    v=\"no\"/>\n";
      print "      <output k=\"moped\"      v=\"no\"/>\n";
      print "      <output k=\"motorbike\"  v=\"no\"/>\n";
      print "      <output k=\"motorcar\"   v=\"no\"/>\n";
      print "      <output k=\"goods\"      v=\"no\"/>\n";
      print "      <output k=\"hgv\"        v=\"no\"/>\n";
      print "      <output k=\"psv\"        v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"oneway\" v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"bridge\" v=\"no\"/>\n";
      print "      <output k=\"tunnel\" v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"bicycleroute\" v=\"no\"/>\n";
      print "    </if>\n";
      print "\n";
     }

   if(m%</relation>%)
     {
      print "    <!-- Special case for walking -->\n";
      print "\n";
      print "    <if>\n";
      print "      <output k=\"restriction\" v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"bicycleroute\" v=\"no\"/>\n";
      print "    </if>\n";
      print "\n";
     }

   print;
  }
