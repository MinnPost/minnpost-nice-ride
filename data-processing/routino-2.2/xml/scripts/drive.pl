#!/usr/bin/perl

while(<STDIN>)
  {
   if(m%</way>%)
     {
      print "    <!-- Special case for motor vehicles -->\n";
      print "\n";
      print "    <if>\n";
      print "      <output k=\"foot\"       v=\"no\"/>\n";
      print "      <output k=\"horse\"      v=\"no\"/>\n";
      print "      <output k=\"wheelchair\" v=\"no\"/>\n";
      print "      <output k=\"bicycle\"    v=\"no\"/>\n";
      print "\n";
      print "      <output k=\"bridge\" v=\"no\"/>\n";
      print "      <output k=\"tunnel\" v=\"no\"/>\n";
      print "    </if>\n";
      print "\n";
     }

   if(m%</relation>%)
     {
      print "    <!-- Special case for motor vehicles -->\n";
      print "\n";
      print "    <if>\n";
      print "      <output k=\"footroute\"    v=\"no\"/>\n";
      print "      <output k=\"bicycleroute\" v=\"no\"/>\n";
      print "    </if>\n";
      print "\n";
     }

   print;
  }
