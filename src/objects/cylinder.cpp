#ifdef WIN32_VC90
#pragma warning (disable : 4251)
#endif

#include "cylinder.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/freeglut.h>

#include <QDebug>

using namespace std;

#include <luabind/operator.hpp>

Cylinder::Cylinder(const btVector3& dim, btScalar mass) {
  init(dim.getX(), dim.getY(), dim.getZ(), mass);
}

Cylinder::Cylinder(btScalar width, btScalar height, btScalar depth,
				   btScalar mass) {

  init(width, height, depth, mass);
}

void Cylinder::init(btScalar width, btScalar height, btScalar depth,
					btScalar mass) {

  lengths[0] = width;
  lengths[1] = height;
  lengths[2] = depth;

  shape = new btCylinderShapeZ(btVector3(width, height, depth*.5));

  btQuaternion qtn;
  btTransform trans;
  btDefaultMotionState *motionState;

  trans.setIdentity();
  qtn.setEuler(0.0, 0.0, 0.0);
  trans.setRotation(qtn);
  trans.setOrigin(btVector3(0,0,0));			
  motionState = new btDefaultMotionState(trans);

  btVector3 inertia;
  shape->calculateLocalInertia(mass,inertia);
  body = new btRigidBody(mass, motionState, shape, inertia);
  
}

void Cylinder::luaBind(lua_State *s) {
  using namespace luabind;

  open(s);

  module(s)
    [
     class_<Cylinder, Object>("Cylinder")
     .def(constructor<>())
     .def(constructor<const btVector3&>())
     .def(constructor<const btVector3&, btScalar>())
     .def(constructor<btScalar, btScalar, btScalar>())
     .def(constructor<btScalar, btScalar, btScalar, btScalar>())
     .def(tostring(const_self))
     ];

  Object::luaBind(s);
}

QString Cylinder::toString() const {
  return QString("Cylinder");
}

void Cylinder::renderInLocalFrame(QTextStream *s) const
{
	
  GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_ambient[] = { color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, 1.0 };
  GLfloat mat_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  // GLfloat no_shininess[] = { 0.0 };
  // GLfloat low_shininess[] = { 5.0 };
  GLfloat high_shininess[] = { 100.0 };
  // GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};
	
  btTransform trans;
  btScalar m[16];

  body->getMotionState()->getWorldTransform(trans);
  trans.getOpenGLMatrix(m);

  glPushMatrix();
  glMultMatrixf(m);
  // translate to match Bullet cylinder origin
  glTranslated(0,0,-lengths[2]*.5);
  glScalef(lengths[0], lengths[1], lengths[2]);
  
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
  glMaterialfv(GL_FRONT, GL_EMISSION, no_mat); 
  glColor3ubv(color);
  
  glutSolidCylinder(1, 1, 16, 16);
  glPopMatrix();

  //TODO  
  if (s != NULL) {
    *s << "cylinder { " << -lengths[2]/2.0 << "*z, " << lengths[2]/2.0 << "*z, " << lengths[0]/2.0 << "\n      pigment { rgb <" << color[0]/255.0 << ", " << color[1]/255.0 << ", " << color[2]/255.0 << "> }" << endl;
    *s <<  "  matrix <" << m[0] << "," << m[1] << "," << m[2] << ",\n        " << m[4] << "," << m[5] << "," << m[6] << ",\n        " << m[8] << "," << m[9] << "," << m[10] << ",\n        " << m[12] << "," << m[13] << "," << m[14] << ">" << endl;
    *s << "}" << endl << endl;
  }
}
