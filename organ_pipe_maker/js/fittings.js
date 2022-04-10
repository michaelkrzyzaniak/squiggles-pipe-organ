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
    var m = new Fitting(div_id, midi_number, material_thickness, normalmensur_deviation, ising_number, is_closed, air_pressure, air_stem_length, tuning_headroom)Fitting(div_id, midi_number, material_thickness, normalmensur_deviation, ising_number, is_closed, air_pressure, air_stem_length, tuning_headroom)
    m.download_stl(filename);
 
  Revision:
    1.0
      Initial release
 
----------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------*/
function Fitting(div_id, length, outer_diameter, material_thickness, inner_oring_depth, inner_oring_height)
{
  this.length = length;
  this.outer_diameter = outer_diameter;
  this.inner_diameter = outer_diameter - 2*material_thickness;
  this.material_thickness = material_thickness;
  this.inner_oring_depth = inner_oring_depth;
  this.inner_oring_height = inner_oring_height;
  this.oring_outer_diameter = 2*inner_oring_depth + this.inner_diameter;
  
  console.log("inner_diameter: " + this.inner_diameter);
  console.log("oring_outer_diameter: " + this.oring_outer_diameter);

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
  controls.autoRotate = true;
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
Fitting.prototype.init = function(div_id)
{


};

/*--------------------------------------------------------------------------------------------*/
Fitting.prototype.plot_and_extrude = function(x, y, depth, opacity)
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
Fitting.prototype.draw_regular_model = function()
{
/*
  this.length = length;
  this.outer_diameter = outer_diameter;
  this.material_thickness = material_thickness;
  this.inner_oring_depth = inner_oring_depth;
  this.inner_oring_height = inner_oring_height;
*/

  /* air inlet pipe */
  
  var outer_cylinder = new THREE.CylinderGeometry(this.outer_diameter*0.5, this.outer_diameter*0.5, this.length, 128);
  var pos_x = 0;
  var pos_y = 0;
  var pos_z = 0;
  outer_cylinder = THREE.CSG.toCSG(outer_cylinder, new THREE.Vector3(pos_x, pos_y, pos_z));

  var inner_cylinder = new THREE.CylinderGeometry(this.inner_diameter*0.5, this.inner_diameter*0.5, this.length, 128);
  pos_x = 0;
  pos_y = 0;
  pos_z = 0;
  inner_cylinder   = THREE.CSG.toCSG(inner_cylinder, new THREE.Vector3(pos_x, pos_y, pos_z));

  var oring_cylinder = new THREE.CylinderGeometry(this.oring_outer_diameter*0.5, this.oring_outer_diameter*0.5, this.inner_oring_height, 128);
  pos_x = 0;
  pos_y = 0;
  pos_z = 0;
  oring_cylinder   = THREE.CSG.toCSG(oring_cylinder, new THREE.Vector3(pos_x, pos_y, pos_z));

  //var model_csg = THREE.CSG.toCSG(mesh.geometry);
  //var geometry  = model_csg.union(outer_cylinder).subtract(inner_cylinder);
  var geometry  = outer_cylinder.subtract(inner_cylinder);
  
  if((this.inner_oring_depth > 0) && (this.inner_oring_height > 0))
    geometry  = geometry.subtract(oring_cylinder)
    
  geometry = THREE.CSG.fromCSG(geometry);
  var material = new THREE.MeshLambertMaterial( { color: 0x0000FF, wireframe:false, opacity: 1, transparent: false});
  var mesh = new THREE.Mesh( geometry, material) ;
  this.scene.add(mesh);

};

/*--------------------------------------------------------------------------------------------*/
Fitting.prototype.scalef = function(x, in_min, in_max, out_min, out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

/*--------------------------------------------------------------------------------------------*/
Fitting.prototype.download_stl = function(filename)
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
