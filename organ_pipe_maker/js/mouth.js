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
   __  __          _   _       _
  |  \/  |___ _  _| |_| |_    (_)___
  | |\/| / _ \ || |  _| ' \ _ | (_-<
  |_|  |_\___/\_,_|\__|_||_(_)/ /__/
                            |__/
 
  Made by Michael Krzyzaniak at Universitetet i Oslo, RITMO Senter for tverrfaglig forskning
  på rytme, tid og bevegelse (Centre for Interdisciplinary Studies in Rhythm, Time and Motion).
  Use at your own risk.
 
  michakrz@ifi.uio.no
  michael.krzyzaniak@yahoo.com
 
  This might very well be the sloppiest piece of code I have ever written, but here
  it is. It works, at least up to the point that the kerf is narrower than the tab
  width, and the inner width is greater than the material thickness.
 
  This is part of the organ pipe maker page. It makes a model of the lower part
  of an organ pipe which can then be downloaded as an STL file for 3d-printing.
  The remainder of the pipe needs to be made with laser-cult plywood (using
  the accompanying resonator module) and inserted  into the top of the  pipe.
  Documentation for all of the parameters is on the index page.
 
  Usage:
    <div id="div_id"></div>
    var m = new Mouth(div_id, midi_number, material_thickness, normalmensur_deviation, ising_number, is_closed, air_pressure, air_stem_length, tuning_headroom)Mouth(div_id, midi_number, material_thickness, normalmensur_deviation, ising_number, is_closed, air_pressure, air_stem_length, tuning_headroom)
    m.download_stl(filename);
 
  Revision:
    1.0
      Initial release
 
----------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------*/
function Mouth(div_id, midi_number, material_thickness, normalmensur_deviation, ising_number, is_closed, air_pressure, air_stem_length, desired_stem_outer_diameter, tuning_headroom)
{
      this.midi_number            = midi_number;
      this.material_thickness     = material_thickness;
      this.normalmensur_deviation = normalmensur_deviation;
      this.ising_number           = ising_number;
      this.is_closed              = is_closed;
      this.air_pressure           = air_pressure;
      this.air_stem_length        = air_stem_length;
      this.tuning_headroom        = tuning_headroom;
  
      this.cps                    = this.midi_2_cps(this.midi_number);
      this.pipe_width             = 155.5 / Math.pow(2, (midi_number + normalmensur_deviation - 36)/16.0);
      this.mouth_height           = this.calculate_mouth_height();
      this.windsheet_width        = this.calculate_windsheet_width(this.mouth_height, this.air_pressure, this.ising_number);
      this.air_flow               = this.calculate_air_flow(this.windsheet_width, this.pipe_width, this.air_pressure);
      this.total_pipe_length      = this.calculate_total_pipe_length(this.midi_number, this.pipe_width, this.is_closed);
      this.tuning_headroom_mm     = this.total_pipe_length - this.calculate_total_pipe_length((this.midi_number+this.tuning_headroom), this.pipe_width, this.is_closed);
      this.plywood_resonator_length  = this.total_pipe_length - this.tuning_headroom_mm; //correction calculated after pipe is drawn
  
      var stem_params             = this.calculate_stem_diameters(desired_stem_outer_diameter, this.pipe_width, this.material_thickness);
      this.stem_inner_diameter    = stem_params.inner;
      this.stem_outer_diameter    = stem_params.outer;
  
      this.scene = new THREE.Scene();
      var div = document.getElementById(div_id);
      var w = parseInt(div.style.width);
      var h = parseInt(div.style.height);
  
      var aspect =  w/h;

      var camera = new THREE.PerspectiveCamera(45/aspect, aspect, 0.1, 1000 );
      camera.position.x = -2; //positive moves the camera to the viewers right
      camera.position.y = 0; //positive moves the camera up
      camera.position.z = 500; //positive is back
      camera.lookAt(  new THREE.Vector3( 0, 0, 0 ) );

      var renderer = new THREE.WebGLRenderer({ antialias: true });
      renderer.setSize( w, h );
      div.appendChild( renderer.domElement );
  
      var controls = new THREE.OrbitControls( camera, renderer.domElement );
      //controls.autoRotate = true;
      var ambient = new THREE.AmbientLight( 0x808080 );
      camera.add(ambient);
 
      var directional_light = new THREE.DirectionalLight(0xffffff);
      directional_light.position.set(-0, 1000, 1000);
  
      light_holder = new THREE.Group();
      light_holder.add(directional_light);
      this.scene.add(light_holder);


      this.scene.background = new THREE.Color( 0xFFFFFF );

      this.plywood_resonator_length -= this.draw_regular_model();

      var animate = function () {
        requestAnimationFrame( animate );
        light_holder.quaternion.copy(camera.quaternion);
        controls.update();
        renderer.render( this.scene, camera );
      }.bind(this);

      animate();
}

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.init = function(div_id)
{


};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.plot_and_extrude = function(x, y, depth, opacity)
{
  var path = new THREE.Shape();
  
  path.moveTo(x[0], y[0]);
  for(var i=1; i<x.length; i++)
    path.lineTo(x[i], y[i]);
  path.lineTo(x[0], y[0]);

  var extrude_settings =
  {
    steps: 2,
    depth: depth,
    bevelEnabled:false,
  };
  
  var geometry = new THREE.ExtrudeGeometry( path, extrude_settings );
  var material = new THREE.MeshLambertMaterial( { color: 0xFFFFFF, wireframe:false, opacity: opacity, transparent: true});
  var mesh = new THREE.Mesh( geometry, material) ;
  this.scene.add(mesh);
  return mesh;
}

/*--------------------------------------------------------------------------------------------*/
//returns the 'active' pipe length, from air outlet to place where resonator connects, in mm.
Mouth.prototype.draw_regular_model = function()
{
  var pipe_plywood_thickness    = this.material_thickness;
  var global_scaling            = this.pipe_width / 40;

  /* fipple_nub_thickness overwritten later */
  var fipple_nub_thickness      = Math.max(5 /*10*/ * global_scaling, pipe_plywood_thickness);
  var fipple_wall_thickness     = pipe_plywood_thickness;
  var l_wall_thickness          = fipple_wall_thickness;
  var r_wall_thickness          = pipe_plywood_thickness;
  
  var floor_height              = pipe_plywood_thickness;//13  * global_scaling;
  var throat_height             = 13  * global_scaling;
  var r_tongue_height           = 9   * global_scaling;
  var l_tongue_height           = 24  * global_scaling;
  var mouth_height              = this.mouth_height;
  var fipple_reed_height        = 25  * global_scaling;

  if(fipple_nub_thickness > pipe_plywood_thickness)
    {
      fipple_reed_height = fipple_reed_height * pipe_plywood_thickness / fipple_nub_thickness;
      fipple_nub_thickness = pipe_plywood_thickness;
    }
  var fipple_nub_height         = 0;//5  * global_scaling;
  var fipple_clearance_height   = pipe_plywood_thickness;//Math.max(20  * global_scaling, fipple_nub_thickness - fipple_wall_thickness);
  var pipe_inset_height         = Math.min(3*pipe_plywood_thickness, 20 * global_scaling);
  var l_tongue_width            = (14 * global_scaling);
  var r_tongue_width            = Math.max(0, (26 * global_scaling) - (this.windsheet_width));
  this.windsheet_width          = this.pipe_width - (l_tongue_width + r_tongue_width);
  
  var r_tongue_flat_thing_width = 3   * global_scaling;
  
  var stem_inner_diameter       = this.stem_inner_diameter; //10;
  var stem_outer_diameter       = this.stem_outer_diameter; //30;
  var stem_height               = this.air_stem_length;
  
  var mesh;

  /* throat, starting at bottom left corner and moving right */
  var x = [];
  var y = [];
  x[0]  = 0;
  y[0]  = 0;
  x[1]  = x[0] + l_wall_thickness + this.windsheet_width + l_tongue_width + r_tongue_width + r_wall_thickness;
  y[1]  = y[0];
  x[2]  = x[1];
  y[2]  = y[1] + floor_height + throat_height + l_tongue_height + mouth_height + fipple_reed_height + fipple_clearance_height + fipple_nub_height;
  
  
  x[3]  = x[2] - r_wall_thickness;
  y[3]  = y[2];
  x[4]  = x[3];
  y[4]  = y[3] - fipple_clearance_height - fipple_reed_height - fipple_nub_height - mouth_height - l_tongue_height + r_tongue_height;
  x[5]  = x[4] - l_tongue_width - r_tongue_width + r_tongue_flat_thing_width;
  y[5]  = y[4] + l_tongue_height - r_tongue_height;
  x[6]  = x[5] - r_tongue_flat_thing_width;
  y[6]  = y[5];
  x[7]  = x[6] + l_tongue_width;
  y[7]  = y[6] - l_tongue_height;
  x[8]  = x[7] + r_tongue_width;
  y[8]  = y[7];
  x[9]  = x[8];
  y[9]  = y[8] - throat_height;
  x[10] = x[9] - l_tongue_width - r_tongue_width - this.windsheet_width;
  y[10] = y[9];
  x[11] = x[10];
  y[11] = y[10] + throat_height + l_tongue_height;
  x[12] = x[11] - l_wall_thickness;
  y[12] = y[11];
  mesh = this.plot_and_extrude(x, y, this.pipe_width, 1);

  /* air inlet pipe */
  var outer_air_inlet_pipe = new THREE.CylinderGeometry(stem_outer_diameter*0.5, stem_outer_diameter*0.5, stem_height, 64);
  var pos_x = this.pipe_width*0.5 + r_wall_thickness;
  var pos_y = -stem_height * 0.5;
  var pos_z = this.pipe_width*0.5;
  var outer_cyl   = THREE.CSG.toCSG(outer_air_inlet_pipe, new THREE.Vector3(pos_x, pos_y, pos_z));

  var inner_air_inlet_pipe = new THREE.CylinderGeometry(stem_inner_diameter*0.5, stem_inner_diameter*0.5, stem_height + floor_height + (0.5 * throat_height), 64);
  pos_x = this.pipe_width*0.5 + r_wall_thickness;
  pos_y = -(stem_height - floor_height) / 2;
  pos_z = this.pipe_width*0.5;
  var inner_cyl   = THREE.CSG.toCSG(inner_air_inlet_pipe, new THREE.Vector3(pos_x, pos_y, pos_z));

  var model_csg = THREE.CSG.toCSG(mesh.geometry);
  var geometry  = model_csg.union(outer_cyl).subtract(inner_cyl);
  mesh.geometry = THREE.CSG.fromCSG(geometry);

  /* fipple, starting at top right and moving down */
  var x_ = [];
  var y_ = [];
  x_[0] = x[10];
  y_[0] = y[2];
  x_[1] = x_[0];
  y_[1] = y_[0] - fipple_clearance_height - fipple_nub_height - fipple_reed_height;
  x_[2] = x_[1] - fipple_nub_thickness;
  y_[2] = y_[1] + fipple_reed_height;
  x_[3] = x_[2];
  y_[3] = y_[2] + fipple_nub_height;
  x_[4] = x_[3] + fipple_nub_thickness - fipple_wall_thickness;
  y_[4] = y_[3] + fipple_nub_thickness - fipple_wall_thickness;
  x_[5] = x_[4];
  y_[5] = y_[4] + fipple_clearance_height - fipple_nub_thickness + fipple_wall_thickness;

  this.plot_and_extrude(x_, y_, this.pipe_width, 1);
  
  /* walls */
  var x__ = [];
  var y__ = [];
  x__[0] = x[0];
  y__[0] = y[0];
  x__[1] = x[1];
  y__[1] = y[1];
  x__[2] = x[2];
  y__[2] = y[2];
  x__[3] = x_[x_.length-1];
  y__[3] = y_[x_.length-1];
  mesh = this.plot_and_extrude(x__, y__, r_wall_thickness, 1);
  mesh.position.z = -r_wall_thickness;
  mesh = this.plot_and_extrude(x__, y__, r_wall_thickness, 0.8);
  mesh.position.z = this.pipe_width;
  
  //back pipe inset
  x__ = [];
  y__ = [];
  x__[0] = x[2];
  y__[0] = y[2] - r_wall_thickness;
  x__[1] = x__[0] + r_wall_thickness;
  y__[1] = y__[0];
  x__[2] = x__[1];
  y__[2] = y__[1] + r_wall_thickness + pipe_inset_height;
  x__[3] = x__[2] - r_wall_thickness;
  y__[3] = y__[2];
  mesh = this.plot_and_extrude(x__, y__, this.pipe_width + r_wall_thickness + r_wall_thickness, 1);
  mesh.position.z = -r_wall_thickness;
  
  //side pipe insets
  x__ = [];
  y__ = [];
  x__[0] = x_[x_.length-1];
  y__[0] = y_[x_.length-1] - r_wall_thickness;
  x__[1] = x[2] + r_wall_thickness;
  y__[1] = y[2] - r_wall_thickness;
  x__[2] = x__[1];
  y__[2] = y__[1] + pipe_inset_height + r_wall_thickness;
  x__[3] = x__[0];
  y__[3] = y__[0] + pipe_inset_height + r_wall_thickness;
  mesh = this.plot_and_extrude(x__, y__, r_wall_thickness, 1);
  mesh.position.z = -2*r_wall_thickness;
  mesh = this.plot_and_extrude(x__, y__, r_wall_thickness, 1);
  mesh.position.z = this.pipe_width + r_wall_thickness;
  
  this.scene.position.x = -(this.pipe_width * 0.5 + r_wall_thickness);
  this.scene.position.y = -(floor_height + throat_height + l_tongue_height + mouth_height + fipple_reed_height + fipple_clearance_height + fipple_nub_height + pipe_inset_height - pipe_plywood_thickness) / 2.0;
  this.scene.position.z = -(this.pipe_width + r_wall_thickness) * 0.5 ;
  
  return fipple_reed_height + fipple_nub_height + fipple_clearance_height + mouth_height;
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.calculate_mouth_height = function()
{
  /* http://www.rwgiangiulio.com/math/mouthheight.htm
     evidently  this is not related to the width of
     the pipe, despite popular belief
  */
  
  var cps = this.midi_2_cps(this.midi_number);
  var result = 0;
  
  if(this.is_closed)
    result = Math.pow(3.018 - 0.233 * Math.log(cps) /* base e */, 5);
  else //open
    result = 550.0 / Math.pow(2, Math.log(cps) /* base e */);
  
  return result;
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.calculate_windsheet_width = function(mouth_height, air_pressure, ising_number)
{
  /* http://www.rwgiangiulio.com/math/wst.htm
  */
  
  if(air_pressure <= 0) return 1  ;
  
  var cps = this.midi_2_cps(this.midi_number);
  
  mouth_height *= 0.039370079; //convert to inches
  air_pressure *= 0.039370079; //convert to inches
  
  
  ising_number *= ising_number;
  cps *= cps;
  mouth_height = mouth_height * mouth_height * mouth_height;
  
  var wst = ising_number * cps * mouth_height / air_pressure;
  wst *= 0.0000394716; //mm

  return wst;
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.calculate_air_flow = function(windsheet_width, pipe_width, air_pressure)
{
  /* http://www.rwgiangiulio.com/math/flowrate.htm
  
    cubic meters per second
  */

  return 0.000004058322226280518 * pipe_width * windsheet_width * Math.sqrt(air_pressure);
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.calculate_total_pipe_length = function(midi_number, pipe_width, is_closed)
{
  /* http://www.rwgiangiulio.com/math/pipelength.htm
  */

  var result;
  
  var cps = this.midi_2_cps(midi_number);
  
  if(is_closed)
    result = (85750.0 / cps) - (pipe_width);
  else
    result = (171500.0 / cps) - (2 * pipe_width);
  
  return result;
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.__calculate_stem_diameters = function(midi_number, pipe_width, material_thickness)
{
  var desired_inner = this.scalef(midi_number, 30, 93, 35, 6);
  desired_inner = Math.min(desired_inner, pipe_width);
  if(desired_inner < 1) desired_inner = 1;
  var desired_outer = desired_inner + 2*material_thickness;
  //var desired_outer = pipe_width + 2*material_thickness;
  var outer_increment = 5; //go in 5mm increments
  desired_outer = outer_increment * Math.ceil(desired_outer / outer_increment);
  desired_inner = desired_outer - 2*material_thickness;
  
  return {inner:desired_inner, outer:desired_outer};
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.calculate_stem_diameters = function(desired_outer, pipe_width, material_thickness)
{
  var desired_inner = desired_outer - 2*material_thickness;
  desired_inner = Math.min(desired_inner, pipe_width);
  if(desired_inner < 1)
    {
       desired_inner = 1;
       desired_outer = desired_inner + 2*material_thickness;
    }
  
  return {inner:desired_inner, outer:desired_outer};
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.scalef = function(x, in_min, in_max, out_min, out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};


/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.midi_2_cps = function(midi_number)
{
  return (440 * Math.pow(2, (midi_number-69) / 12.0));
};

/*--------------------------------------------------------------------------------------------*/
Mouth.prototype.download_stl = function(filename)
{
  var exporter = new THREE.STLExporter();
  var str = exporter.parse(this.scene);
  var blob = new Blob( [str], { type : 'text/plain' } );
  var link = document.createElement('a');
  link.style.display = 'none';
  document.body.appendChild(link);
  link.href = URL.createObjectURL(blob);
  link.download = filename + ".stl";
  link.click();
  document.body.removeChild(link);
};
