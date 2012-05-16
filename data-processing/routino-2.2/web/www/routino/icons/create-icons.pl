#!/usr/bin/perl
#
# Routino icons Perl script
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2010 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

use Graphics::Magick;

# Markers for routing

@names=("red","grey");
@borders=("black","grey");
@letters=("red","grey");

foreach $character ('0'..'9','home')
  {
   foreach $colour (0..$#names)
     {
      $image=Graphics::Magick->new;
      $image->Set(size => "63x75");

      $image->ReadImage('xc:white');
      $image->Transparent('white');

      $image->Draw(primitive => polygon, points => '1,32 32,73 61,32 32,10',
                   stroke => $borders[$colour], fill => 'white', strokewidth => 6,
                   antialias => 'false');

      $image->Draw(primitive => arc,     points => '1,1 61,61 -180,0',
                   stroke => $borders[$colour], fill => 'white', strokewidth => 6,
                   antialias => 'false');

      if($character eq 'home')
        {
         $home=Graphics::Magick->new;

         $home->ReadImage("home.png");

         $home->Opaque(fill => $names[$colour], color => 'black');

         $image->Composite(image => $home, compose => Over,
                           x => 32-$home->Get('width')/2, y => 26-$home->Get('height')/2);
        }
      else
        {
         ($x_ppem, $y_ppem, $ascender, $descender, $width, $height, $max_advance) = 
           $image->QueryFontMetrics(text => $character, font => 'Helvetica', pointsize => '36');

         $image->Annotate(text => $character, font => 'Helvetica', pointsize => '36',
                          stroke => $letters[$colour], fill => $letters[$colour],
                          x => 32, y => 32-$descender, align => Center,
                          antialias => 'false');
        }

      $image->Resize(width => 21, height => 25);

      $image->Write("marker-$character-$names[$colour].png");

      undef $image;
     }
  }

# Balls for visualiser descriptions

@colours=("#FFFFFF",
          "#FF0000",
          "#FFFF00",
          "#00FF00",
          "#8B4513",
          "#00BFFF",
          "#FF69B4",
          "#000000",
          "#000000",
          "#000000");

foreach $colour (0..9)
  {
   $image=Graphics::Magick->new;
   $image->Set(size => "9x9");

   $image->ReadImage('xc:white');
   $image->Transparent('white');

   $image->Draw(primitive => circle, points => '4,4 4,8',
                fill => $colours[$colour], stroke => $colours[$colour],
                antialias => 'false');

   $image->Write("ball-$colour.png");

   undef $image;
  }

# Limit signs

foreach $limit (1..160)
  {
   &draw_limit($limit);
  }

foreach $limit (10..200)
  {
   &draw_limit(sprintf "%.1f",$limit/10);
  }

&draw_limit("no");

unlink "limit-0.png";
link "limit-no.png","limit-0.png";

unlink "limit-0.0.png";
link "limit-no.png","limit-0.0.png";

sub draw_limit
  {
   ($limit)=@_;

   $image=Graphics::Magick->new;
   $image->Set(size => "57x57");

   $image->ReadImage('xc:white');
   $image->Transparent('white');

   $image->Draw(primitive => circle, points => '28,28 28,55',
                stroke => 'red', fill => 'white', strokewidth => 3,
                antialias => 'false');

   ($x_ppem, $y_ppem, $ascender, $descender, $width, $height, $max_advance) =
     $image->QueryFontMetrics(text => "$limit", font => 'Helvetica', pointsize => '22');

   $image->Annotate(text => "$limit", font => 'Helvetica', pointsize => '22',
                    stroke => 'black', fill => 'black',
                    x => 28, y => 28-$descender, align => Center,
                    antialias => 'false');

   $image->Resize(width => 19, height => 19);

   $image->Write("limit-$limit.png");

   undef $image;
  }
