--
-- spur gears - generated with OpenSCAD
--
-- http://www.thingiverse.com/thing:6894
--

v.gravity = btVector3(0,-980.1, 0)

plane = Plane(0,1,0,0,10000)
plane.col = "#707070"
v:add(plane)

c = Cube(160,120,10,0)
c.pos=btVector3(0,60,0)
v:add(c)

-- small gear

g2 = Mesh("demo/stl/spur-02.stl", 1)
g2.col = "#ff0f00"
g2.pos = btVector3(50,60,6)
v:add(g2)

pivot0 = btVector3(50,0,6)
axis0  = btVector3(0,0,1)

pivot1 = btVector3(0,0,0)
axis1  = btVector3(0,0,1)

con0 = btHingeConstraint(
  c.body, g2.body,
  pivot0, pivot1, axis0, axis1)

speed = 6
s1 = 500

con0:enableAngularMotor(true, speed, s1)

v:addConstraint(con0)

-- big center gear

g1 = Mesh("demo/stl/spur-01.stl", 5)
g1.pos = btVector3(0,60,6)
g1.col = "#ff0f00"
v:add(g1)

pivot0 = btVector3(0,0,6)
axis0  = btVector3(0,0,1)

pivot1 = btVector3(0,0,0)
axis1  = btVector3(0,0,1)

con1 = btHingeConstraint(
  c.body, g1.body,
  pivot0, pivot1, axis0, axis1)

v:addConstraint(con1)

g3 = Mesh("demo/stl/HighlyConfigurableWheelv1.stl", 5)
g3.col = "#00ff00"
g3.pos = btVector3(0,60,32)
v:add(g3)

pivot3 = btVector3(0,0,26)

con2 = btHingeConstraint(
  g1.body, g3.body,
  pivot3, pivot1, axis0, axis1)

con2:enableAngularMotor(true, 0, 1000)

v:addConstraint(con2)

--print(v.cam.look)
v.cam.look =  btVector3(0,60,0)
--print(v.cam.look)