-- demo of the transform.lua library

col = require "color"
t   = require "transform"

plane = Plane(0,1,0,0,1000)
plane.col = col.green
v:add(plane)

v.gravity = btVector3(0,-9.81*20,0)

X = 50
Y = 50

mt = {} -- create the matrix
for i=1,X do
  for j=1,Y do
  c = Cube(0.75,0.75,0.75,0)
  c.col= "#707070"
  t.rotate(c, btQuaternion(1,0,1,1), btVector3(i/X,0,0))
  t.move  (c, btVector3(i-X/2,0,j-Y/2))
  v:add(c)
  mt[i*Y + j] = c
  end
end

function update(N)
  for i=1,X do
    for j=1,Y do
      tmp = mt[i*Y + j].pos
      p1 = math.sin(N/100)*10
      p2 = math.cos(N/100)*10
      tmp.y = math.cos(p2*2+i*.2)*math.cos(p1+j*.2)*2 + 4
      mt[i*Y + j].pos = tmp
    end
  end
end

update(0)

v:preSim(function(N)

if (math.fmod(N, 30) == 0) then
if (math.random() > 0.2) then
  s=Sphere(2,1)
  s.col = col.red
else
  s=Cube(3,3,3,1)
  s.col = color.random_pastel()
end
  t.move(s, btVector3(0,0,0))
  t.move(s, btVector3(0,10,0))
  v:add(s)
end

update(N)

end)