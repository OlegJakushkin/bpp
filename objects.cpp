#include "objects.h"
#include <GL/glut.h>

#include <QDebug>

using namespace std;

#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>

Objects::Objects() {
}

void Objects::luaBind(lua_State *s) {
  using namespace luabind;

  open(s);

  module(s)
    [
     class_<Objects, Object>("Objects")
     .def(constructor<>(), adopt(result))
	 .def(tostring(const_self))
	 ];
}

QList<Object *> Objects::getObjects() const {
}

QString Objects::toString() const {
  return QString("Objects");
}

void Objects::renderInLocalFrame(QTextStream *s) const {
  qDebug() << "Objects::renderInLocalFrame should not be called!";
}
