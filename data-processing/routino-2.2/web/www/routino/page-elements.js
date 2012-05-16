//
// Javascript for page elements.
//
// Part of the Routino routing software.
//
// This file Copyright 2008,2009 Andrew M. Bishop
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


//
// Display one of the tabs and associated DIV and hide the others
//

function tab_select(name)
{
 var tab=document.getElementById("tab_" + name);

 if(tab.className=="tab_selected")
   {return;}

 // Hide the deslected tabs and DIVs

 var parent=tab.parentNode;
 var child=parent.firstChild;

 do
   {
    if(String(child.id).substr(0,4)=="tab_")
      {
       var div=document.getElementById(child.id + "_div");

       child.className="tab_unselected";
       div.style.display="none";
      }

    child=child.nextSibling;
   }
 while(child!=null);

 // Display the newly selected tab and DIV

 var div=document.getElementById(tab.id + "_div");

 tab.className="tab_selected";
 div.style.display="";
}


//
// Show the associated DIV
//

function hideshow_show(name)
{
 var span1=document.getElementById("hideshow_" + name + "_show");
 var span2=document.getElementById("hideshow_" + name + "_hide");
 var div=document.getElementById("hideshow_" + name + "_div");

 div.style.display="";
 span1.className="hideshow_hide";
 span2.className="hideshow_show";
}


//
// Hide the associated DIV
//

function hideshow_hide(name)
{
 var span1=document.getElementById("hideshow_" + name + "_show");
 var span2=document.getElementById("hideshow_" + name + "_hide");
 var div=document.getElementById("hideshow_" + name + "_div");

 div.style.display="none";
 span2.className="hideshow_hide";
 span1.className="hideshow_show";
}
