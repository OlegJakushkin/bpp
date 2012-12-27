-- *********************
-- DRIVEABLE CAR EXAMPLE
-- *********************
--
-- NOTES:
-- + Please turn deactivation OFF or keyboad shortcuts will not work
--
-- KEYBOARD SHORTCUTS:
-- + F1  = switch camera 
-- + F2  = accelerate
-- + F3  = neutral
-- + F4  = reverse
-- + F9  = turn left
-- + F10 = turn right

-- *******
-- CONTROL
-- *******

-- camera
camera_type=1

-- chassis specs: Nissan Micra (by Ren Bui)
chassis_height=8.25
front_axe_xpos=11.8
rear_axe_xpos=-12.7
front_axe_width=13.9
rear_axe_width=13.9
front_arm_lenght=3.75
rear_arm_lenght=3.75
arm_rad=.25
wheel_rad=2.8
wheel_width=1.8
disc_width=.25
spring_softness=.1
wheel_mass=150
arm_mass=350
disc_mass=150
car_body_mass=8200-wheel_mass*4-arm_mass*4-disc_mass*4-arm_mass*.5

-- engine specs: Nissan Micra
angular_motor_speed=15
angular_motor_power=550
car_previous_velocity=btVector3(angular_motor_speed*0,0,0)
-- drive type : 0=NONE, 1=FWD, 2=RWD, 3=AWD
drive_type=0

-- generic tire params
tire_friction=.7
tire_restitution=.9

-- obstacles: 
-- 0=NONE, 1=ALTERNATE, 2=STEPS, 3=RAMP, 4=MESH TERRAIN, 5=urban 6=posts
use_obstacles=0

-- reference ground marks
use_markings=1


-- *****
-- FLOOR
-- *****
if(use_obstacles~=4) then
plane = Plane(0,1,0)
plane.col = "#111111"
plane.friction=0.9
plane.restitution=.1
plane.pre_sdl="object{track"
v:add(plane)
end
-- STABILITY TEST STAND (comment out the plane too)
stand=Cube(17,1,10,0)
stand.pos=btVector3(-1,2.5,0)
--v:add(stand)


-- ********
-- CAR MESH
-- ********
car_body=Mesh3DS("demo/micraK11_Rene_Bui.3ds",car_body_mass)
car_body.pos =btVector3(0,chassis_height,0)
car_body.col="#003399"
car_body.vel=car_previous_velocity
car_body.restitution=.01
car_body.friction=.1
car_body.pre_sdl="object{car"
v:add(car_body)


-- **********
-- FRONT ARMS
-- **********

-- left
front_left_arm=Cylinder(arm_rad,arm_rad,front_arm_lenght,arm_mass)
front_left_arm.pos=btVector3(front_axe_xpos,wheel_rad,front_axe_width*.5-wheel_width*.5-disc_width-front_arm_lenght*.5)
front_left_arm.vel=car_previous_velocity
front_left_arm.pre_sdl="object{front_arm"
v:add(front_left_arm)

-- right
front_right_arm=Cylinder(arm_rad,arm_rad,front_arm_lenght,arm_mass)
front_right_arm.pos=btVector3(front_axe_xpos,wheel_rad,-front_axe_width*.5+wheel_width*.5+disc_width+front_arm_lenght*.5)
front_right_arm.vel=car_previous_velocity
front_right_arm.pre_sdl="object{front_arm"
v:add(front_right_arm)


-- ***********
-- FRONT DISCS
-- ***********

-- left
front_left_disc=Cylinder(wheel_rad*.5,wheel_rad*.5,disc_width,disc_mass)
front_left_disc.pos=btVector3(front_axe_xpos,wheel_rad,front_axe_width*.5-wheel_width*.5-disc_width*.5)
front_left_disc.vel=car_previous_velocity
front_left_disc.pre_sdl="object{front_disc"
v:add(front_left_disc)

-- right
front_right_disc=Cylinder(wheel_rad*.5,wheel_rad*.5,disc_width,disc_mass)
front_right_disc.pos=btVector3(front_axe_xpos,wheel_rad,-(front_axe_width*.5-wheel_width*.5-disc_width*.5))
front_right_disc.vel=car_previous_velocity
front_right_disc.pre_sdl="object{front_disc"
v:add(front_right_disc)


-- ************
-- FRONT WHEELS
-- ************

-- left
front_left_wheel = Cylinder(wheel_rad,wheel_rad,wheel_width,wheel_mass)
front_left_wheel.pos = btVector3(front_axe_xpos, wheel_rad, front_axe_width*.5)
front_left_wheel.col = "#333333"
front_left_wheel.friction=tire_friction
front_left_wheel.restitution=tire_restitution
front_left_wheel.vel=car_previous_velocity
front_left_wheel.pre_sdl="object{gs_tyre"
v:add(front_left_wheel)

-- right
front_right_wheel = Cylinder(wheel_rad,wheel_rad,wheel_width,wheel_mass)
front_right_wheel.pos = btVector3(front_axe_xpos, wheel_rad, -front_axe_width*.5)
front_right_wheel.col = "#333333"
front_right_wheel.friction=tire_friction
front_right_wheel.restitution=tire_restitution
front_right_wheel.vel=car_previous_velocity
front_right_wheel.pre_sdl="object{gs_tyre"
v:add(front_right_wheel)


-- ******************
-- FRONT STEERING BAR
-- ******************

front_steering_bar=Cylinder(arm_rad*.5,arm_rad*.5,front_axe_width-wheel_width-disc_width*2,arm_mass*.5)
front_steering_bar.pos=btVector3(front_axe_xpos+wheel_rad*.4,wheel_rad,0)
v:add(front_steering_bar)


-- ************************************
-- FRONT STEERING BAR-DISCS CONSTRAINTS
-- ************************************

-- left
front_left_disc_pivot = btVector3(wheel_rad*.4,0,-disc_width*.5)
left_steering_bar_pivot = btVector3(0,0,front_axe_width*.5-wheel_width*.5-disc_width)
left_steering_bar_constraint = btPoint2PointConstraint(
  front_left_disc:getRigidBody(),
  front_steering_bar:getRigidBody(),
  front_left_disc_pivot,
  left_steering_bar_pivot
)
v:addConstraint(left_steering_bar_constraint)

-- right
front_right_disc_pivot = btVector3(wheel_rad*.4,0,disc_width*.5)
right_steering_bar_pivot = btVector3(0,0,-front_axe_width*.5+wheel_width*.5+disc_width)
right_steering_bar_constraint = btPoint2PointConstraint(
  front_right_disc:getRigidBody(),
  front_steering_bar:getRigidBody(),
  front_right_disc_pivot,
  right_steering_bar_pivot
)
v:addConstraint(right_steering_bar_constraint)


-- ******************************************
-- FRONT WHEEL-DISC CONSTRAINTS, ENGINE HINGE
-- *******************************************

-- left
front_left_disc_pivot = btVector3(0,0,disc_width*.5)
front_left_disc_axis  = btVector3(0,0,1)
front_left_wheel_pivot = btVector3(0,0,-wheel_width*.5)
front_left_wheel_axis  = btVector3(0,0,1)
front_left_wheel_disc_constraint = btHingeConstraint(
  front_left_disc:getRigidBody(),
  front_left_wheel:getRigidBody(),
  front_left_disc_pivot, front_left_wheel_pivot, front_left_disc_axis, front_left_wheel_axis
)
if (drive_type==1 or drive_type==3) then
  front_left_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
end
v:addConstraint(front_left_wheel_disc_constraint)

-- right
front_right_disc_pivot = btVector3(0,0,-disc_width*.5)
front_right_disc_axis  = btVector3(0,0,1)
front_right_wheel_pivot = btVector3(0,0,wheel_width*.5)
front_right_wheel_axis  = btVector3(0,0,1)
front_right_wheel_disc_constraint = btHingeConstraint(
  front_right_disc:getRigidBody(),
  front_right_wheel:getRigidBody(),
  front_right_disc_pivot, front_right_wheel_pivot, front_right_disc_axis, front_right_wheel_axis
)
if (drive_type==1 or drive_type==3) then
  front_right_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
end
v:addConstraint(front_right_wheel_disc_constraint)


-- **************************************
-- FRONT DISC-ARM CONSTRAINT, STEER HINGE
-- **************************************

-- left
front_left_arm_pivot = btVector3(0,0,front_arm_lenght*.5)
front_left_arm_axis  = btVector3(0,0,1)
front_left_disc_pivot = btVector3(0,0,-disc_width*.5)
front_left_disc_axis  = btVector3(0,0,1)
front_left_disc_arm_constraint = btHingeConstraint(
  front_left_arm:getRigidBody(),
  front_left_disc:getRigidBody(),
  front_left_arm_pivot, front_left_disc_pivot, front_left_arm_axis, front_left_disc_axis)
front_left_disc_arm_constraint:setAxis(btVector3(0,1,0))
--front_left_disc_arm_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(front_left_disc_arm_constraint)

-- right
front_right_arm_pivot = btVector3(0,0,-front_arm_lenght*.5)
front_right_arm_axis  = btVector3(0,0,1)
front_right_disc_pivot = btVector3(0,0,disc_width*.5)
front_right_disc_axis  = btVector3(0,0,1)
front_right_disc_arm_constraint = btHingeConstraint(
  front_right_arm:getRigidBody(),
  front_right_disc:getRigidBody(),
  front_right_arm_pivot, front_right_disc_pivot, front_right_arm_axis, front_right_disc_axis)
front_right_disc_arm_constraint:setAxis(btVector3(0,1,0))
--front_right_disc_arm_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(front_right_disc_arm_constraint)


-- *******************************************
-- FRONT ARM-BODY CONSTRAINT, SUSPENSION HINGE
-- *******************************************

-- left
front_left_arm_pivot = btVector3(0,0,-front_arm_lenght*.5)
front_left_arm_axis  = btVector3(0,0,1)
front_left_body_pivot = btVector3(0,wheel_rad-chassis_height,front_axe_width*.5-wheel_width*.5-disc_width-front_arm_lenght)
front_left_body_axis  = btVector3(0,0,1)
front_left_arm_body_constraint = btHingeConstraint(
  front_left_arm:getRigidBody(),
  car_body:getRigidBody(),
  front_left_arm_pivot, front_left_body_pivot, front_left_arm_axis, front_left_body_axis)
front_left_arm_body_constraint:setAxis(btVector3(1,0,0))
--front_left_arm_body_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(front_left_arm_body_constraint)

-- right
front_right_arm_pivot = btVector3(0,0,front_arm_lenght*.5)
front_right_arm_axis  = btVector3(0,0,1)
front_right_body_pivot = btVector3(0,wheel_rad-chassis_height,-(front_axe_width*.5-wheel_width*.5-disc_width-front_arm_lenght))
front_right_body_axis  = btVector3(0,0,1)
front_right_arm_body_constraint = btHingeConstraint(
  front_right_arm:getRigidBody(),
  car_body:getRigidBody(),
  front_right_arm_pivot, front_right_body_pivot, front_right_arm_axis, front_right_body_axis) 
front_right_arm_body_constraint:setAxis(btVector3(-1,0,0))
--front_right_arm_body_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(front_right_arm_body_constraint)


-- ********************************************
-- FRONT ARM-BODY CONSTRAINT, SUSPENSION SPRING
-- ********************************************

q = btQuaternion()
q:setRotation(btVector3(0,0,1),math.pi*.5)

-- left
o = btVector3(0,0,front_arm_lenght*.5-disc_width)
front_left_arm_trans=btTransform(q,o)
o = btVector3(front_axe_xpos,-1,front_axe_width*.5-wheel_width*.5-disc_width*2)
front_left_body_trans=btTransform(q,o)
front_left_body_arm_constraint=btSliderConstraint(
  front_left_arm:getRigidBody(),
  car_body:getRigidBody(),
  front_left_arm_trans,
  front_left_body_trans,
  false
)
front_left_body_arm_constraint:setSoftnessOrthoLin(spring_softness)
front_left_body_arm_constraint:setSoftnessLimLin(spring_softness)
v:addConstraint(front_left_body_arm_constraint)

-- right
o = btVector3(0,0,-front_arm_lenght*.5+disc_width)
front_right_arm_trans=btTransform(q,o)
o = btVector3(front_axe_xpos,-1,-(front_axe_width*.5-wheel_width*.5-disc_width*2))
front_right_body_trans=btTransform(q,o)
front_right_body_arm_constraint=btSliderConstraint(
  front_right_arm:getRigidBody(),
  car_body:getRigidBody(),
  front_right_arm_trans,
  front_right_body_trans,
  false
)
front_right_body_arm_constraint:setSoftnessOrthoLin(spring_softness)
front_right_body_arm_constraint:setSoftnessLimLin(spring_softness)
v:addConstraint(front_right_body_arm_constraint)


-- *********
-- REAR ARMS
-- *********

-- left
rear_left_arm=Cylinder(arm_rad,arm_rad,front_arm_lenght,arm_mass)
rear_left_arm.pos=btVector3(rear_axe_xpos,wheel_rad,rear_axe_width*.5-wheel_width*.5-disc_width-front_arm_lenght*.5)
rear_left_arm.vel=car_previous_velocity
rear_left_arm.pre_sdl="object{rear_arm"
v:add(rear_left_arm)

-- right
rear_right_arm=Cylinder(arm_rad,arm_rad,rear_arm_lenght,arm_mass)
rear_right_arm.pos=btVector3(rear_axe_xpos,wheel_rad,-rear_axe_width*.5+wheel_width*.5+disc_width+rear_arm_lenght*.5)
rear_right_arm.vel=car_previous_velocity
rear_right_arm.pre_sdl="object{rear_arm"
v:add(rear_right_arm)


-- **********
-- REAR DISCS
-- **********

-- left
rear_left_disc=Cylinder(wheel_rad*.5,wheel_rad*.5,disc_width,disc_mass)
rear_left_disc.pos=btVector3(rear_axe_xpos,wheel_rad,rear_axe_width*.5-wheel_width*.5-disc_width*.5)
rear_left_disc.vel=car_previous_velocity
rear_left_disc.pre_sdl="object{rear_disc"
v:add(rear_left_disc)

-- right
rear_right_disc=Cylinder(wheel_rad*.5,wheel_rad*.5,disc_width,disc_mass)
rear_right_disc.pos=btVector3(rear_axe_xpos,wheel_rad,-(rear_axe_width*.5-wheel_width*.5-disc_width*.5))
rear_right_disc.vel=car_previous_velocity
rear_right_disc.pre_sdl="object{rear_disc"
v:add(rear_right_disc)


-- ***********
-- REAR WHEELS
-- ***********

-- left
rear_left_wheel = Cylinder(wheel_rad,wheel_rad,wheel_width,wheel_mass)
rear_left_wheel.pos = btVector3(rear_axe_xpos, wheel_rad, rear_axe_width*.5)
rear_left_wheel.col = "#333333"
rear_left_wheel.friction=tire_friction
rear_left_wheel.restitution=tire_restitution
rear_left_wheel.vel=car_previous_velocity
rear_left_wheel.pre_sdl="object{gs_tyre"
v:add(rear_left_wheel)

-- right
rear_right_wheel = Cylinder(wheel_rad,wheel_rad,wheel_width,wheel_mass)
rear_right_wheel.pos = btVector3(rear_axe_xpos, wheel_rad, -rear_axe_width*.5)
rear_right_wheel.col = "#333333"
rear_right_wheel.friction=tire_friction
rear_right_wheel.restitution=tire_restitution
rear_right_wheel.vel=car_previous_velocity
rear_right_wheel.pre_sdl="object{gs_tyre"
v:add(rear_right_wheel)


-- *******************
-- REAR STABILIZER BAR
-- *******************

rear_steering_bar=Cylinder(arm_rad*.5,arm_rad*.5,rear_axe_width-wheel_width-disc_width*2,arm_mass*.5)
rear_steering_bar.pos=btVector3(rear_axe_xpos+wheel_rad*.4,wheel_rad,0)
v:add(rear_steering_bar)


-- *************************************
-- REAR STABILIZER BAR-DISCS CONSTRAINTS
-- *************************************

-- left
rear_left_disc_pivot = btVector3(wheel_rad*.4,0,-disc_width*.5)
rear_left_steering_bar_pivot = btVector3(0,0,rear_axe_width*.5-wheel_width*.5-disc_width)
rear_left_steering_bar_constraint = btPoint2PointConstraint(
  rear_left_disc:getRigidBody(),
  rear_steering_bar:getRigidBody(),
  rear_left_disc_pivot,
  rear_left_steering_bar_pivot
)
v:addConstraint(rear_left_steering_bar_constraint)

-- right
rear_right_disc_pivot = btVector3(wheel_rad*.4,0,disc_width*.5)
rear_right_steering_bar_pivot = btVector3(0,0,-rear_axe_width*.5+wheel_width*.5+disc_width)
rear_right_steering_bar_constraint = btPoint2PointConstraint(
  rear_right_disc:getRigidBody(),
  rear_steering_bar:getRigidBody(),
  rear_right_disc_pivot,
  rear_right_steering_bar_pivot
)
v:addConstraint(rear_right_steering_bar_constraint)

-- center
rear_center_body_pivot = btVector3(rear_axe_xpos+wheel_rad*.4,-chassis_height+wheel_rad,0)
rear_center_steering_bar_pivot = btVector3(0,0,0)
rear_center_steering_bar_constraint = btPoint2PointConstraint(
  car_body:getRigidBody(),
  rear_steering_bar:getRigidBody(),
  rear_center_body_pivot,
  rear_center_steering_bar_pivot
)
v:addConstraint(rear_center_steering_bar_constraint)


-- *****************************************
-- REAR WHEEL-DISC CONSTRAINTS, ENGINE HINGE
-- *****************************************

-- left
rear_left_disc_pivot = btVector3(0,0,disc_width*.5)
rear_left_disc_axis  = btVector3(0,0,1)
rear_left_wheel_pivot = btVector3(0,0,-wheel_width*.5)
rear_left_wheel_axis  = btVector3(0,0,1)
rear_left_wheel_disc_constraint = btHingeConstraint(
  rear_left_disc:getRigidBody(),
  rear_left_wheel:getRigidBody(),
  rear_left_disc_pivot, rear_left_wheel_pivot, rear_left_disc_axis, rear_left_wheel_axis
)
if (drive_type==2 or drive_type==3) then
  rear_left_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
end
v:addConstraint(rear_left_wheel_disc_constraint)

-- right
rear_right_disc_pivot = btVector3(0,0,-disc_width*.5)
rear_right_disc_axis  = btVector3(0,0,1)
rear_right_wheel_pivot = btVector3(0,0,wheel_width*.5)
rear_right_wheel_axis  = btVector3(0,0,1)
rear_right_wheel_disc_constraint = btHingeConstraint(
  rear_right_disc:getRigidBody(),
  rear_right_wheel:getRigidBody(),
  rear_right_disc_pivot, rear_right_wheel_pivot, rear_right_disc_axis, rear_right_wheel_axis
)
if (drive_type==2 or drive_type==3) then
  rear_right_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
end
v:addConstraint(rear_right_wheel_disc_constraint)


-- **************************************
-- REAR DISC-ARM CONSTRAINT, STEER HINGE
-- **************************************

-- left
rear_left_arm_pivot = btVector3(0,0,rear_arm_lenght*.5)
rear_left_arm_axis  = btVector3(0,0,1)
rear_left_disc_pivot = btVector3(0,0,-disc_width*.5)
rear_left_disc_axis  = btVector3(0,0,1)
rear_left_disc_arm_constraint = btHingeConstraint(
  rear_left_arm:getRigidBody(),
  rear_left_disc:getRigidBody(),
  rear_left_arm_pivot, rear_left_disc_pivot, rear_left_arm_axis, rear_left_disc_axis)
rear_left_disc_arm_constraint:setAxis(btVector3(0,1,0))
rear_left_disc_arm_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(rear_left_disc_arm_constraint)

-- right
rear_right_arm_pivot = btVector3(0,0,-rear_arm_lenght*.5)
rear_right_arm_axis  = btVector3(0,0,1)
rear_right_disc_pivot = btVector3(0,0,disc_width*.5)
rear_right_disc_axis  = btVector3(0,0,1)
rear_right_disc_arm_constraint = btHingeConstraint(
  rear_right_arm:getRigidBody(),
  rear_right_disc:getRigidBody(),
  rear_right_arm_pivot, rear_right_disc_pivot, rear_right_arm_axis, rear_right_disc_axis)
rear_right_disc_arm_constraint:setAxis(btVector3(0,1,0))
rear_right_disc_arm_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(rear_right_disc_arm_constraint)


-- *******************************************
-- REAR ARM-BODY CONSTRAINT, SUSPENSION HINGE
-- *******************************************

-- left
rear_left_arm_pivot = btVector3(0,0,-rear_arm_lenght*.5)
rear_left_arm_axis  = btVector3(0,0,1)
rear_left_body_pivot = btVector3(0,wheel_rad-chassis_height,rear_axe_width*.5-wheel_width*.5-disc_width-rear_arm_lenght)
rear_left_body_axis  = btVector3(0,0,1)
rear_left_arm_body_constraint = btHingeConstraint(
  rear_left_arm:getRigidBody(),
  car_body:getRigidBody(),
  rear_left_arm_pivot, rear_left_body_pivot, rear_left_arm_axis, rear_left_body_axis)
rear_left_arm_body_constraint:setAxis(btVector3(1,0,0))
--rear_left_arm_body_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(rear_left_arm_body_constraint)

-- right
rear_right_arm_pivot = btVector3(0,0,rear_arm_lenght*.5)
rear_right_arm_axis  = btVector3(0,0,1)
rear_right_body_pivot = btVector3(0,wheel_rad-chassis_height,-(rear_axe_width*.5-wheel_width*.5-disc_width-rear_arm_lenght))
rear_right_body_axis  = btVector3(0,0,1)
rear_right_arm_body_constraint = btHingeConstraint(
  rear_right_arm:getRigidBody(),
  car_body:getRigidBody(),
  rear_right_arm_pivot, rear_right_body_pivot, rear_right_arm_axis, rear_right_body_axis) 
rear_right_arm_body_constraint:setAxis(btVector3(-1,0,0))
--rear_right_arm_body_constraint:setLimit(0,0,.1,.3,1)
v:addConstraint(rear_right_arm_body_constraint)


-- ********************************************
-- REAR ARM-BODY CONSTRAINT, SUSPENSION SPRING
-- ********************************************

q = btQuaternion()
q:setRotation(btVector3(0,0,1),math.pi*.5)

-- left
o = btVector3(0,0,rear_arm_lenght*.5-disc_width)
rear_left_arm_trans=btTransform(q,o)
o = btVector3(rear_axe_xpos,-1,rear_axe_width*.5-wheel_width*.5-disc_width*2)
rear_left_body_trans=btTransform(q,o)
rear_left_body_arm_constraint=btSliderConstraint(
  rear_left_arm:getRigidBody(),
  car_body:getRigidBody(),
  rear_left_arm_trans,
  rear_left_body_trans,
  false
)
rear_left_body_arm_constraint:setSoftnessOrthoLin(spring_softness)
rear_left_body_arm_constraint:setSoftnessLimLin(spring_softness)
v:addConstraint(rear_left_body_arm_constraint)

-- right
o = btVector3(0,0,-rear_arm_lenght*.5+disc_width)
rear_right_arm_trans=btTransform(q,o)
o = btVector3(rear_axe_xpos,-1,-(rear_axe_width*.5-wheel_width*.5-disc_width*2))
rear_right_body_trans=btTransform(q,o)
rear_right_body_arm_constraint=btSliderConstraint(
  rear_right_arm:getRigidBody(),
  car_body:getRigidBody(),
  rear_right_arm_trans,
  rear_right_body_trans,
  false
)
rear_right_body_arm_constraint:setSoftnessOrthoLin(spring_softness)
rear_right_body_arm_constraint:setSoftnessLimLin(spring_softness)
v:addConstraint(rear_right_body_arm_constraint)


-- ************
-- TEST SCENERY
-- ************

-- ground markings for reference of deviation
if(use_markings==1) then
 if(use_obstacles<4) then
  for i = 1,80 do
   marks=Cube(5,.01,1,0)
   marks.pos=btVector3(-400+i*10,.005,20)
   marks.col = "#FF9900"
   marks.friction=.9
   marks.pre_sdl="object{track_mark()"
   v:add(marks)
  end
  for i = 1,80 do
   marks=Cube(5,.01,1,0)
   marks.pos=btVector3(-400+i*10,.005,-20)
   marks.col = "#FF9900"
   marks.friction=.9
   marks.pre_sdl="object{track_mark()"
   v:add(marks)
  end
  for i = 1,80 do
   marks=Cube(5,.01,1,0)
   marks.pos=btVector3(-400+i*10,.005,60)
   marks.col = "#FF9900"
   marks.friction=.9
   marks.pre_sdl="object{track_mark()"
   v:add(marks)
  end
  for i = 1,80 do
   marks=Cube(5,.01,1,0)
   marks.pos=btVector3(-400+i*10,.005,-60)
   marks.col = "#FF9900"
   marks.friction=.9
   marks.pre_sdl="object{track_mark()"
   v:add(marks)
  end
 end
end

-- suspension test: alternate obstacles
if(use_obstacles==1) then
 for i = 1,80 do
  obs1=Cube(1,1,15,0)
  if(i%2==0) then
    obs1.pos=btVector3(20+i*5,.5,7.5)
  else
    obs1.pos=btVector3(20+i*5,.5,-7.5)
  end
  obs1.col = "#FF9900"
  obs1.friction=.9
  v:add(obs1)
 end
end

-- suspension test: steps
if(use_obstacles==2) then
 for i = 1,40 do
  obs1=Cube(10,1,30,0)
  if(i<=20) then
    obs1.pos=btVector3(20+i*10,.5+i,0)
  else
    obs1.pos=btVector3(20+i*10,.5+40-i,0)
  end
  obs1.col = "#FF9900"
  obs1.friction=.9
  v:add(obs1)
 end
end

-- suspension test: ramp
if(use_obstacles==3) then
 ramp=Cube(30,.1,30,0)
 q = btQuaternion(0,0,.1,1)
 o = btVector3(150,3,0)
 ramp.trans=btTransform(q,o)
 ramp.pre_sdl="object{ramp"
 v:add(ramp)
end

-- suspension test: mesh terrain test
if(use_obstacles==4) then
  terrain=Mesh3DS("demo/terrain.3ds",0)
  terrain.pos =btVector3(430,4,0)
  terrain.col="#993399"
  terrain.friction=.9
  terrain.restitution=0
  v:add(terrain)
end


-- ******************
-- KEYBOARD SHORTCUTS
-- ******************

-- steer left
v:addShortcut("F9", function(N)
  front_left_disc_arm_constraint:enableAngularMotor(true, 100, -500)
  front_right_disc_arm_constraint:enableAngularMotor(true, 100, -500)
end)

-- steer right
v:addShortcut("F10", function(N)
  front_right_disc_arm_constraint:enableAngularMotor(true, 100, 500)
  front_left_disc_arm_constraint:enableAngularMotor(true, 100, 500)
end)

-- accelerate forward
v:addShortcut("F2", function(N)
  front_left_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
  front_right_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
  rear_left_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
  rear_right_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, angular_motor_power)
end)

-- neutral
v:addShortcut("F3", function(N)
  front_left_wheel_disc_constraint:enableAngularMotor(false, 0, 0)
  front_right_wheel_disc_constraint:enableAngularMotor(false, 0,0)
  rear_left_wheel_disc_constraint:enableAngularMotor(false, 0,0)
  rear_right_wheel_disc_constraint:enableAngularMotor(false, 0,0)
end)

-- accelerate backward
v:addShortcut("F4", function(N)
  front_left_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, -angular_motor_power)
  front_right_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, -angular_motor_power)
  rear_left_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, -angular_motor_power)
  rear_right_wheel_disc_constraint:enableAngularMotor(true, angular_motor_speed, -angular_motor_power)
end)


-- *************************
-- on-the-fly camera control
-- *************************
v:postSim(function(N)

  cam = Cam()
  -- pseudo orthographic from the side
  if(camera_type==0) then
  cam.pos = btVector3(car_body.pos.x,10,car_body.pos.z+2000)
  cam.look = btVector3(car_body.pos.x,10,car_body.pos.z)
  cam:setHorizontalFieldOfView(.025)
  end
  -- overview 1
  if(camera_type==1) then
  cam.pos = btVector3(car_body.pos.x+40,40,car_body.pos.z+50)
  cam.look = car_body.pos
  end
  -- overview 2
  if(camera_type==2) then
  cam.pos = btVector3(50,40,car_body.pos.z+30)
  cam.look = car_body.pos
  cam:setHorizontalFieldOfView(1)
  end
  -- look at the front suspension and wheels
  if(camera_type==3) then
  cam.pos = btVector3(car_body.pos.x+99,car_body.pos.y+14,car_body.pos.z)
  cam.look = btVector3(car_body.pos.x,car_body.pos.y,car_body.pos.z)
  cam:setHorizontalFieldOfView(.5)
  end
  -- look at the rear suspension and wheels
  if(camera_type==4) then
  cam.pos = btVector3(car_body.pos.x-45,car_body.pos.y-3,car_body.pos.z)
  cam.look = btVector3(car_body.pos.x+10-30,car_body.pos.y-3,car_body.pos.z)
  end
  -- onboard
  if(camera_type==5) then
  cam.pos = btVector3(car_body.pos.x,car_body.pos.y+4,car_body.pos.z)
  cam.look = btVector3(front_steering_bar.pos.x,car_body.pos.y+4,front_steering_bar.pos.z)
  end
  -- from below
  if(camera_type==6) then
  cam.pos = btVector3(car_body.pos.x,car_body.pos.y-400,car_body.pos.z)
  cam.look = btVector3(car_body.pos.x,car_body.pos.y,car_body.pos.z)
  cam:setHorizontalFieldOfView(.1)
  end
  v:cam(cam)
  
end) -- postSim

-- EOF