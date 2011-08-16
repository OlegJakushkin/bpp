#include "sphere.h"
#include <GL/glut.h>

#include <QDebug>

using namespace std;

Sphere::Sphere(btScalar pradius, btScalar mass)
{
  radius = pradius;

  shape = new btSphereShape(radius);

  btQuaternion qtn;
  btTransform trans;
  btDefaultMotionState *motionState;

  trans.setIdentity();
  qtn.setEuler(0.0, 0.0, 0.0);
  trans.setRotation(qtn);
  trans.setOrigin(btVector3(0, 0, 0));
  motionState = new btDefaultMotionState(trans);

  body = new btRigidBody(mass, motionState, shape, btVector3(mass, mass, mass));
}

void Sphere::setRadius(btScalar pradius) {
  delete shape;

  radius = pradius;

  shape = new btSphereShape(radius);
}

void Sphere::renderInLocalFrame(QTextStream *s) const
{
  GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_ambient[] = { color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, 1.0 };
  GLfloat mat_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat no_shininess[] = { 0.0 };
  GLfloat low_shininess[] = { 5.0 };
  GLfloat high_shininess[] = { 100.0 };
  GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};

  btTransform trans;
  btScalar m[16];

  glPushMatrix();

  //glDisable(GL_COLOR_MATERIAL);

  body->getMotionState()->getWorldTransform(trans);
  trans.getOpenGLMatrix(m);

  /*
  m[ 0] = r[ 0];m[ 1] = r[ 4];m[ 2] = r[ 8];m[ 3] = 0;
  m[ 4] = r[ 1];m[ 5] = r[ 5];m[ 6] = r[ 9];m[ 7] = 0;
  m[ 8] = r[ 2];m[ 9] = r[ 6];m[10] = r[10];m[11] = 0;
  m[12] = p[ 0];m[13] = p[ 1];m[14] = p[ 2];m[15] = 1;
  */

  glMultMatrixf(m);
  
  glScalef(radius, radius, radius);

  //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
  glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

  //glColor3ubv(color);

  glutSolidSphere(1.0f, 16, 16);
  glPopMatrix();

  if (s != NULL) {
    *s << "sphere { <.0,.0,.0>, " << radius << endl;
    
    if (mTexture == NULL) {
      *s << "      pigment { rgb <" << color[0]/255.0 << ", " << color[1]/255.0 << ", " << color[2]/255.0 << "> }" << endl;
    } else {
      *s << mTexture << endl;
    }

    *s <<  "  matrix <" << m[0] << "," << m[1] << "," << m[2] << ",\n        " << m[4] << "," << m[5] << "," << m[6] << ",\n        " << m[8] << "," << m[9] << "," << m[10] << ",\n        " << m[12] << "," << m[13] << "," << m[14] << ">" << endl;


    if (mFinish != NULL) {
      *s << mFinish << endl;
    }

    *s << "}" << endl << endl;
  }
}
