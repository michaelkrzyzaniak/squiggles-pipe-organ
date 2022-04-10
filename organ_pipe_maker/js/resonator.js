/*----------------------------------------------------------------------------------------------
         _          __________                              _,
     _.-(_)._     ."          ".      .--""--.          _.-{__}-._
   .'________'.   | .--------. |    .'        '.      .:-'`____`'-:.
  [____________] /` |________| `\  /   .'``'.   \    /_.-"`_  _`"-._\
  /  / .\/. \  \|  / / .\/. \ \  ||  .'/.\/.\'.  |  /`   / .\/. \   `\
  |  \__/\__/  |\_/  \__/\__/  \_/|  : |_/\_| ;  |  |    \__/\__/    |
  \            /  \            /   \ '.\    /.' / .-\                /-.
  /'._  --  _.'\  /'._  --  _.'\   /'. `'--'` .'\/   '._-.__--__.-_.'   \
 /_   `""""`   _\/_   `""""`   _\ /_  `-./\.-'  _\'.    `""""""""`    .'`\
(__/    '|    \ _)_|           |_)_/            \__)|        '       |   |
  |_____'|_____|   \__________/   |              |;`_________'________`;-'
   '----------'    '----------'   '--------------'`--------------------`
------------------------------------------------------------------------------------------------
   ___                       _             _
  | _ \___ ___ ___ _ _  __ _| |_ ___ _ _  (_)___
  |   / -_|_-</ _ \ ' \/ _` |  _/ _ \ '_| | (_-<
  |_|_\___/__/\___/_||_\__,_|\__\___/_|(_)/ /__/
                                        |__/
 
  Made by Michael Krzyzaniak at Universitetet i Oslo, RITMO Senter for tverrfaglig forskning
  på rytme, tid og bevegelse (Centre for Interdisciplinary Studies in Rhythm, Time and Motion).
  Use at your own risk.
 
  michakrz@ifi.uio.no
  michael.krzyzaniak@yahoo.com
 
  This might very well be the sloppiest piece of code I have ever written, but here
  it is. It works, at least up to the point that the kerf is narrower than the tab
  width, and the inner width is greater than the material thickness.
 
  This is part  of the organ pipe maker page. It makes an svg file that can be
  downloaded and laser-cut out of plywood, and constitutes the top part of the organ
  pipe. It makes a rectangular cross-section tube that fits into the 3d-printed
  mouth of the pipe. It also makes a tuning slide which can be open (no lid)
  or closed (yes lid) that is meant to be lined with felt and slid over the
  organ  pipe to tune it down to the correct pitch. This also makes a rectangle
  that can be used to laser-cut the felt for the tuning slide.
 
  Usage:
    <div id="div_id"></div>
    var r = new Resonator(div_id, inner_width, inner_height, nominal_tab_width, kerf, material_width, felt_width, is_closed);
    r.download_svg(filename);
 
  Revision:
    1.0
      Initial release
 
----------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------*/
function Resonator(div_id, inner_width, inner_height, nominal_tab_width, kerf, material_width, felt_width, is_closed)
{
  this.div_id = div_id;
  var clearance        = Math.max(1, 2 * kerf);
  var x_offset         = clearance;
  var svg = "<svg width='400mm' height='300mm' viewBox='0 0 400 300' version='1.1'xmlns='http://www.w3.org/2000/svg'>";

  svg += this.draw_two_sides_of_a_box(x_offset, inner_width, inner_height, nominal_tab_width, kerf, material_width, false, clearance, false, false);
  x_offset += 2 * (clearance + inner_width + 2*material_width);

  svg += this.draw_two_sides_of_a_box(x_offset, inner_width, inner_height, nominal_tab_width, kerf, material_width, false, clearance, false, false);
  x_offset += 2 * (clearance + inner_width  + 2*material_width);

  inner_height *= 0.5;
  inner_width += 2 * (felt_width + material_width);

  svg += this.draw_two_sides_of_a_box(x_offset, inner_width, inner_height, nominal_tab_width, kerf, material_width, is_closed, clearance, false, true);
  x_offset += (clearance + inner_width  + 2*material_width);
  svg += this.draw_two_sides_of_a_box(x_offset, inner_width, inner_height, nominal_tab_width, kerf, material_width, is_closed, clearance, is_closed, true);
  x_offset += (clearance + inner_width + 2*material_width);
  
  inner_width -= felt_width;
  var y_offset = (is_closed) ? inner_width+ 2*(kerf+felt_width + clearance + material_width) : clearance;
  svg += this.draw_rectangle(x_offset, y_offset, inner_height, 4*inner_width, kerf, clearance);

  svg += "</svg>";

  document.getElementById(div_id).innerHTML = svg;
}

/*--------------------------------------------------------------------------------------------*/
Resonator.prototype.draw_rectangle = function(x_offset, y_offset, inner_width, inner_height, kerf, clearance)
{
  inner_width += kerf;
  inner_height += kerf;
  var svg = "<path d='M" + x_offset + " " + y_offset + "v" + inner_height + "h" + inner_width + "v" + -inner_height + "Z' />";
  return svg;
}

/*--------------------------------------------------------------------------------------------*/
Resonator.prototype.draw_two_sides_of_a_box = function(x_offset, inner_width, inner_height, nominal_tab_width, kerf, material_width, is_closed, clearance, needs_lid, draw_below)
{
  var num_tabs          = Math.ceil(inner_height / nominal_tab_width);
  if((num_tabs % 2) == 0) ++num_tabs;
  if(num_tabs < 3) num_tabs = 3;
  var actual_tab_width  = inner_height / num_tabs;
  var outie_tab_width   = actual_tab_width + kerf;
  var innie_tab_width   = actual_tab_width - kerf;
  
  var part_width       = inner_width + 2*material_width;
  var num_lid_tabs      = Math.ceil(part_width / nominal_tab_width);
  if((num_lid_tabs % 2) == 0) ++num_lid_tabs;
  if(num_lid_tabs < 3) num_lid_tabs = 3;
  var actual_lid_tab_width = part_width / num_lid_tabs;
  var outie_lid_tab_width = actual_lid_tab_width + kerf;
  var innie_lid_tab_width = actual_lid_tab_width - kerf;
  
  var i, j;
  var svg;

  //Start each path at top left corner and draw down
  svg = "<path d='M" + x_offset + " " + clearance + "v" + outie_tab_width;
  for(i=0; i<Math.floor(num_tabs/2); i++)
    {
      svg += "h" +  material_width + "v" + innie_tab_width;
      svg += "h" + -material_width + "v" + outie_tab_width;
    }
  
  if(is_closed)
    {
      svg += "h" +  (innie_lid_tab_width + kerf);
      svg += "v" +  material_width + "h" + outie_lid_tab_width;
    
      for(i=0; i<Math.floor((num_lid_tabs-2)/2); i++)
        {
          svg += "v" + -material_width + "h" + innie_lid_tab_width;
          svg += "v" +  material_width + "h" + outie_lid_tab_width;
        }
      svg += "v" + -material_width + "h" + (innie_lid_tab_width + kerf);
    }
  else //is_open
    svg += "h" + (part_width + kerf);
  
  svg += "v" + -outie_tab_width
  for(i=0; i<Math.floor(num_tabs/2); i++)
    {
      svg += "h" + -material_width + "v" + -innie_tab_width;
      svg += "h" + material_width  + "v" + -outie_tab_width;
    }
  svg += "Z' />"
  
  if(draw_below)
    {
      svg += "<path d='M" + (x_offset+material_width) + " " + (2*clearance+inner_height+2*kerf+material_width) + "v" + actual_tab_width;
    }
  else
    {
      x_offset += clearance + part_width;
      svg += "<path d='M" + (x_offset+material_width) + " " + clearance + "v" + actual_tab_width;
    }
  for(i=0; i<Math.floor(num_tabs/2); i++)
    {
      svg += "h" + -material_width + "v" + outie_tab_width;
      svg += "h" + material_width + "v" + innie_tab_width;
    }
  
  svg += "v" + kerf;
  if(is_closed)
    {
      svg += "h" + (innie_lid_tab_width-material_width+kerf) + "v" + material_width; //kerf?
      svg += "h" + outie_lid_tab_width;
      for(i=0; i<Math.floor((num_lid_tabs-2)/2); i++)
        {
          svg += "v" + -material_width + "h" + innie_lid_tab_width;
          svg += "v" +  material_width + "h" + outie_lid_tab_width;
        }
      svg += "v" + -material_width + "h" + (innie_lid_tab_width-material_width+kerf);
    }
  else //is_open
    svg += "h" + (inner_width+kerf);
  
  svg += "v" + -actual_tab_width;
  for(i=0; i<Math.floor(num_tabs/2); i++)
    {
      svg += "h" + material_width + "v" + -outie_tab_width;
      svg += "h" + -material_width  + "v" + -innie_tab_width;
    }
  svg += "v" + -kerf;
  svg += "Z' />";
  
  if(needs_lid)
    {
      //draw lid
      x_offset += clearance + part_width;
      svg += "<path d='M" + x_offset + " " + clearance + "v" + outie_lid_tab_width;
      for(i=0; i<Math.floor((num_lid_tabs)/2); i++)
      {
         svg += "h" +  material_width + "v" + innie_lid_tab_width;
         svg += "h" + -material_width + "v" + outie_lid_tab_width;
      }
    
      svg += "h" +  outie_lid_tab_width;
      for(i=0; i<Math.floor((num_lid_tabs)/2); i++)
      {
         svg += "v" + -material_width + "h" + innie_lid_tab_width;
         svg += "v" +  material_width + "h" + outie_lid_tab_width;
      }

      svg += "v" + -outie_lid_tab_width;
      for(i=0; i<Math.floor((num_lid_tabs)/2); i++)
      {
         svg += "h" + -material_width + "v" + -innie_lid_tab_width;
         svg += "h" +  material_width + "v" + -outie_lid_tab_width;
      }

      svg += "h" + -outie_lid_tab_width;
      for(i=0; i<Math.floor((num_lid_tabs)/2); i++)
      {
         svg += "v" +  material_width + "h" + -innie_lid_tab_width;
         svg += "v" + -material_width + "h" + -outie_lid_tab_width;
      }
      svg += "Z' />";
    }
  
  return svg;
}

/*--------------------------------------------------------------------------------------------*/
Resonator.prototype.download_svg = function(name)
{
  var svg =
  "<?xml version='1.0' standalone='no'?>" +
  "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>";
  
  svg += document.getElementById(this.div_id).innerHTML;
  
  var blob = new Blob([svg], {type: 'image/svg+xml'});

  var link_id = "svg_download_link";
  var a = document.getElementById(link_id);
  if(!a)
    {
      var a = document.createElement("a");
      a.id  = link_id;
      a.style = "display: none";
      document.body.appendChild(a);
    }

  var url    = window.URL.createObjectURL(blob);
  a.href     = url;
  a.download = name + ".svg";
  a.click();
  window.URL.revokeObjectURL(url);
}
