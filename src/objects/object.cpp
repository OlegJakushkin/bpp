#ifdef WIN32_VC90
#pragma warning (disable : 4251)
#endif

#include "object.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glut.h>

#include <QDebug>

#include <boost/shared_ptr.hpp>
#include <luabind/adopt_policy.hpp>

using namespace std;

std::ostream& operator<<(std::ostream& ostream, const Object& obj) {
  ostream << obj.toString().toAscii().data() << "[ rgb: " <<
	obj.getColor().red() << ", " << obj.getColor().green() << ", " <<
	obj.getColor().blue() << "]";

  return ostream;
}

#include <luabind/operator.hpp>

Object::Object(QObject *parent) : QObject(parent) {
  shape = 0;
  body = 0;

  photons_enable = false;
  photons_reflection = false;
  photons_refraction = false;

  setCollisionTypes(COL_WALL, COL_WALL);
}

Object::~Object() {

  //  qDebug() << "Object::~Object()";

  if (shape != NULL) {
	delete shape;
	shape = NULL;
  }

  if (body != NULL) {
    delete body->getMotionState();
	delete body;
	body = NULL;
  }
}

void Object::setCollisionTypes(collisiontypes col1, collisiontypes col2) {
  this->col1 = col1;
  this->col2 = col2;
}

collisiontypes Object::getCol1() const {
  return col1;
}

collisiontypes Object::getCol2() const {
  return col2;
}

QList<btTypedConstraint *> Object::getConstraints() {
  return _constraints;
}

QString Object::toString() const {
  return QString("Object");
}

void Object::luaBind(lua_State *s) {
  using namespace luabind;

  open(s);

  module(s)
    [
     class_<Object>("Object")
     .def(constructor<>(), adopt(result))
     .def("setColor", (void(Object::*)(int, int, int))&Object::setColor)

	 .property("color",
			   (QColor(Object::*)(void))&Object::getColor,
			   (void(Object::*)(QColor))&Object::setColor)

	 .property("col",
			   (QString(Object::*)(void))&Object::getColorString,
			   (void(Object::*)(QString))&Object::setColorString)

	 .property("pos",
			   (btVector3(Object::*)(void))&Object::getPosition,
               (void(Object::*)(const btVector3&))&Object::setPosition)

	 .property("trans",
			   (btTransform(Object::*)(void))&Object::getTransform,
               (void(Object::*)(const btTransform&))&Object::setTransform)

	 .property("mass",
			   (btScalar(Object::*)(void))&Object::getMass,
			   (void(Object::*)(btScalar))&Object::setMass)

	 .property("vel",
               (btVector3(Object::*)(void))&Object::getLinearVelocity,
               (void(Object::*)(btVector3&))&Object::setLinearVelocity)

	 .property("friction",
			   (btScalar(Object::*)(void))&Object::getFriction,
			   (void(Object::*)(btScalar))&Object::setFriction)

	 .property("restitution",
			   (btScalar(Object::*)(void))&Object::getRestitution,
			   (void(Object::*)(btScalar))&Object::setRestitution)

	 .property("damp_lin",
			   (btScalar(Object::*)(void))&Object::getLinearDamping,
			   (void(Object::*)(btScalar))&Object::setLinearDamping)

	 .property("damp_ang",
			   (btScalar(Object::*)(void))&Object::getAngularDamping,
			   (void(Object::*)(btScalar))&Object::setAngularDamping)

	 // povray properties

	 .property("texture",
			   (QString(Object::*)(void))&Object::getTexture,
			   (void(Object::*)(QString))&Object::setTexture)

	 .property("finish",
			   (QString(Object::*)(void))&Object::getFinish,
			   (void(Object::*)(QString))&Object::setFinish)

	 .property("scale",
			   (QString(Object::*)(void))&Object::getScale,
			   (void(Object::*)(QString))&Object::setScale)

	 .def(tostring(const_self))
	 ];
}

void Object::renderInLocalFrame(QTextStream *s) const {
  Q_UNUSED(s);

  qDebug() << "Object::renderInLocalFrame";
}

void Object::setTexture(QString texture) {
  mTexture = texture;
}

QString Object::getTexture() const {
  return mTexture;
}

void Object::setScale(QString scale) {
  mScale = scale;
}

QString Object::getScale() const {
  return mScale;
}

void Object::setFinish(QString finish) {
  mFinish = finish;
}

QString Object::getFinish() const {
  return mFinish;
}

void Object::setMass(btScalar _mass) {
  body->setMassProps(_mass, btVector3(0,0,0));
  body->updateInertiaTensor();
}

void Object::setFriction(btScalar friction) {
  body->setFriction(friction);
}

btScalar Object::getFriction() const {
  return body->getFriction();
}

void Object::setRestitution(btScalar restitution) {
  body->setRestitution(restitution);
}

btScalar Object::getRestitution() const {
  return body->getRestitution();
}

void Object::setLinearDamping(btScalar linearDamping) {
  body->setDamping(linearDamping, getAngularDamping());
}

void Object::setAngularDamping(btScalar angularDamping) {
  body->setDamping(getLinearDamping(), angularDamping);
}

void Object::setDamping(btScalar linearDamping, btScalar angularDamping) {
  body->setDamping(linearDamping, angularDamping);
}

btScalar Object::getLinearDamping() const {
  return body->getLinearDamping();
}

btScalar Object::getAngularDamping() const {
  return body->getAngularDamping();
}

void Object::setLinearVelocity(const btVector3 &vector) {
  body->setLinearVelocity(vector);
}

btVector3 Object::getLinearVelocity() const {
  return body->getLinearVelocity();
}

void Object::setColor(int r, int g, int b) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
}

void Object::setColor(QColor col) {
  setColor(col.red(), col.green(), col.blue());
}

QColor Object::getColor() const {
  return QColor(color[0], color[1], color[2]);
}

void Object::setColorString(QString col) {
  setColor(QColor(col));
}

QString Object::getColorString() const {
  return getColor().name();
}

void Object::setPosition(const btVector3& v) {
  setPosition(v[0], v[1], v[2]);
}

void Object::setPosition(btScalar x, btScalar y, btScalar z) {
  if (body != NULL) {
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    trans.setOrigin(btVector3(x, y, z));
    delete body->getMotionState();
    body->setMotionState(new btDefaultMotionState(trans));
  }
}

btVector3 Object::getPosition() const {
  btTransform trans;
  body->getMotionState()->getWorldTransform(trans);
  return trans.getOrigin();
}

void Object::setRotation(const btVector3& axis, btScalar angle) {
  if (body != NULL) {
    btTransform trans;
    btQuaternion rot;
    body->getMotionState()->getWorldTransform(trans);
    rot = trans.getRotation();
    rot.setRotation(axis, angle);
    trans.setRotation(rot);
    delete body->getMotionState();
    body->setMotionState(new btDefaultMotionState(trans));
  }
}

void Object::setRotation(const btQuaternion &r) {
  if (body != NULL) {
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    trans.setRotation(r);
    delete body->getMotionState();
    body->setMotionState(new btDefaultMotionState(trans));
  }
}

btQuaternion Object::getRotation() const {
  btTransform trans;
  body->getMotionState()->getWorldTransform(trans);
  return trans.getRotation();
}

void Object::setTransform(const btTransform &trans) {
  if (body != NULL) {
    delete body->getMotionState();
    body->setMotionState(new btDefaultMotionState(trans));
  } 
}

btTransform Object::getTransform() const {
  btTransform trans;
  body->getMotionState()->getWorldTransform(trans);
  return trans;
}

btScalar Object::getMass() const {
  return body->getInvMass();
}

void Object::setPovPhotons(bool _photons_enable,
			   bool _photons_reflection,
			   bool _photons_refraction) {
  photons_enable = _photons_enable;
  photons_reflection = _photons_reflection;
  photons_refraction = _photons_refraction;
}

QString Object::getPovPhotons() const {
  QString tmp("");

  if (photons_enable) {
    tmp += "  photons {\n    target\n    reflection ";
    tmp += (photons_reflection) ? "on" : "off";
    tmp += "\n    refraction ";
    tmp += (photons_refraction) ? "on" : "off";
    tmp += "\n  }\n";
  }

  return tmp;
}


void Object::render(QTextStream *s)
{

  if (body != NULL)
	renderInLocalFrame(s);
  else
	qDebug() << "body == null";
}
