#ifdef WIN32_VC90
#pragma warning (disable : 4251)
#endif

#include "viewer.h"

#include <QColor>

#include "lua_converters.h"

#include "lua_bullet.h"

#ifdef HAS_LUA_QT
#include "lua_register.h"
#endif

#include "objects/object.h"
#include "objects/objects.h"
#include "objects/plane.h"
#include "objects/cube.h"
#include "objects/sphere.h"
#include "objects/cylinder.h"
#include "objects/mesh3ds.h"

#include "objects/palette.h"

#include "objects/cam.h"
#ifdef HAS_QEXTSERIAL
#include "qserial.h"
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glut.h>

#include <QDebug>

#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/info.hpp>

#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/tag_function.hpp>

typedef boost::error_info<struct tag_stack_str,std::string> stack_info;

using namespace std;

std::ostream& operator<<(std::ostream& ostream, const Viewer& v) {
  ostream << v.toString().toAscii().data();
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const QString& s) {
  ostream << s.toAscii().data();
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const QColor& c) {
  ostream << "QColor(\"" << c.name().toAscii().data() << "\")";
  return ostream;
}

QString Viewer::toString() const {
  return QString("Viewer");
}

void Viewer::luaBind(lua_State *s) {
  using namespace luabind;

  open(s);

  module(s)
  [
   class_<Viewer>("Viewer")
   .def(constructor<>())
   .def("add", (void(Viewer::*)(Object *))&Viewer::addObject, adopt(_2))
   .def("remove", (void(Viewer::*)(Object *))&Viewer::removeObject, adopt(luabind::result))
   .def("addConstraint", (void(Viewer::*)(btTypedConstraint *))&Viewer::addConstraint, adopt(_2))
   .def("removeConstraint", (void(Viewer::*)(btTypedConstraint *))&Viewer::removeConstraint, adopt(_2))
   .def("createVehicleRaycaster", &Viewer::createVehicleRaycaster, adopt(luabind::result))
   .def("addVehicle", (void(Viewer::*)(btRaycastVehicle *))&Viewer::addVehicle, adopt(_2))
   .def("addShortcut", &Viewer::addShortcut, adopt(luabind::result))
   .def("removeShortcut", &Viewer::removeShortcut, adopt(luabind::result))
   .def("cam", (void(Viewer::*)(Cam *))&Viewer::setCamera, adopt(_2))
   .def("preDraw", (void(Viewer::*)(const luabind::object &fn))&Viewer::setCBPreDraw, adopt(luabind::result))
   .def("postDraw", (void(Viewer::*)(const luabind::object &fn))&Viewer::setCBPostDraw, adopt(luabind::result))
   .def("preSim", (void(Viewer::*)(const luabind::object &fn))&Viewer::setCBPreSim, adopt(luabind::result))
   .def("postSim", (void(Viewer::*)(const luabind::object &fn))&Viewer::setCBPostSim, adopt(luabind::result))
   .def("preStop", (void(Viewer::*)(const luabind::object &fn))&Viewer::setCBPreStop, adopt(luabind::result))
   .def("onCommand", (void(Viewer::*)(const luabind::object &fn))&Viewer::setCBOnCommand, adopt(luabind::result))
   .def("savePrefs", &Viewer::setPrefs)
   .def("loadPrefs", &Viewer::getPrefs)
   
   .property("gravity", &Viewer::getGravity, &Viewer::setGravity)

   // http://bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_the_World
   .property("timeStep", &Viewer::getTimeStep, &Viewer::setTimeStep)
   .property("maxSubSteps", &Viewer::getMaxSubSteps, &Viewer::setMaxSubSteps)
   .property("fixedTimeStep", &Viewer::getFixedTimeStep, &Viewer::setFixedTimeStep)
   
   .property("glShininess", &Viewer::getGLShininess, &Viewer::setGLShininess)
   .property("glSpecularColor", &Viewer::getGLSpecularColor, &Viewer::setGLSpecularColor)
   .property("glSpecularColor", &Viewer::getGLSpecularCol, &Viewer::setGLSpecularCol)
   .property("glLight0", &Viewer::getGLLight0, &Viewer::setGLLight0)
   .property("glLight1", &Viewer::getGLLight1, &Viewer::setGLLight1)

   .property("glAmbient", &Viewer::getGLAmbient, &Viewer::setGLAmbient)
   .property("glDiffuse", &Viewer::getGLDiffuse, &Viewer::setGLDiffuse)
   .property("glSpecular", &Viewer::getGLSpecular, &Viewer::setGLSpecular)
   .property("glModelAmbient", &Viewer::getGLModelAmbient, &Viewer::setGLModelAmbient)

   .property("glAmbient", &Viewer::getGLAmbientPercent, &Viewer::setGLAmbientPercent)
   .property("glDiffuse", &Viewer::getGLDiffusePercent, &Viewer::setGLDiffusePercent)
   .property("glSpecular", &Viewer::getGLSpecularPercent, &Viewer::setGLSpecularPercent)
   .property("glModelAmbient", &Viewer::getGLModelAmbientPercent, &Viewer::setGLModelAmbientPercent)

   .property("pre_sdl", &Viewer::getPreSDL, &Viewer::setPreSDL)
   .property("post_sdl", &Viewer::getPostSDL, &Viewer::setPostSDL)
   .def(tostring(const_self))
  ];

  // QT helper classes

  module(s)
    [
     class_<QColor>("QColor")
     .def(constructor<>())
     .def(constructor<QString>())
     .def(constructor<int, int, int>())
     .def(constructor<int, int, int, int>())
     .property("r", &QColor::red, &QColor::setRed)
     .property("g", &QColor::green, &QColor::setGreen)
     .property("b", &QColor::blue, &QColor::setBlue)
   .def(tostring(self))
     ];

  module(s)
    [
     class_<QString>("QString")
     .def(constructor<>())
     .def(constructor<const char *>())
     .def(tostring(self))
     ];
}

void Viewer::addObject(Object* o) {
  addObject(o, o->getCol1(), o->getCol2());
  addConstraints(o->getConstraints());
}

void Viewer::removeObject(Object* o) {
  if (o->body != NULL)
    dynamicsWorld->removeRigidBody(o->body);

  _objects->remove(o);
}

void Viewer::addConstraint(btTypedConstraint *con) {
  dynamicsWorld->addConstraint(con, true);
  _constraints->insert(con);
}

void Viewer::removeConstraint(btTypedConstraint *con) {
  dynamicsWorld->removeConstraint(con);
  _constraints->remove(con);
}

void Viewer::addConstraints(QList<btTypedConstraint *> cons) {
  for (int i = 0; i < cons.size(); ++i) {
    addConstraint(cons[i]);
  }
}

btVehicleRaycaster* Viewer::createVehicleRaycaster() {
  return new btDefaultVehicleRaycaster(dynamicsWorld);
}

void Viewer::addVehicle(btRaycastVehicle *veh) {
  dynamicsWorld->addVehicle(veh);
  _raycast_vehicles->insert(veh);
}

void Viewer::luaBindInstance(lua_State *s) {
  using namespace luabind;

  globals(s)["v"] = this;
}

void report_errors(lua_State *L, int status)
{
  if ( status!=0 ) {
    std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1); // remove error message
  }
}

#define G 9.81f

using namespace qglviewer;

namespace {
void getAABB(QSet<Object *> *objects, btScalar aabb[6]) {
  btVector3 aabbMin, aabbMax;

  aabb[0] = -10; aabb[1] = -10; aabb[2] = -10;
  aabb[3] = 10; aabb[4] = 10; aabb[5] = 10;

  QSet<Object*>::iterator oi;
  for (oi = objects->begin(); oi != objects->end(); oi++) {
    Object *o = *oi;

    if (o->toString() != QString("Plane") && o->body) {
      btVector3 oaabbmin, oaabbmax;
      o->body->getAabb(oaabbmin, oaabbmax);

      // qDebug() << o->toString() << oaabbmin.x() << oaabbmin.y() << oaabbmin.z()
      //         << oaabbmax.x() << oaabbmax.y() << oaabbmax.z();

      for (int i = 0; i < 3; ++i) {
        aabb[  i] = qMin(aabb[  i], oaabbmin[  i]);
        aabb[3+i] = qMax(aabb[3+i], oaabbmax[3+i]);
      }
      }
  }

  // qDebug() << "getAABB()" << aabb[0] << aabb[1] << aabb[2] << aabb[3] << aabb[4] << aabb[5];
}
}

void Viewer::keyPressEvent(QKeyEvent *e) {
  int keyInt = e->key();
  Qt::Key key = static_cast<Qt::Key>(keyInt);

  if (key == Qt::Key_unknown) {
      qDebug() << "Unknown key from a macro probably";
      return;
  }

  // the user have clicked just and only the special keys Ctrl, Shift, Alt, Meta.
  if(key == Qt::Key_Control ||
      key == Qt::Key_Shift ||
      key == Qt::Key_Alt ||
      key == Qt::Key_Meta)
  {
      // qDebug() << "Single click of special key: Ctrl, Shift, Alt or Meta";
      // qDebug() << "New KeySequence:" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
      // return;
  }

  // check for a combination of user clicks
  Qt::KeyboardModifiers modifiers = e->modifiers();
  QString keyText = e->text();
  // if the keyText is empty than it's a special key like F1, F5, ...
  //  qDebug() << "Pressed Key:" << keyText;

  QList<Qt::Key> modifiersList;
  if(modifiers & Qt::ShiftModifier)
      keyInt += Qt::SHIFT;
  if(modifiers & Qt::ControlModifier)
      keyInt += Qt::CTRL;
  if(modifiers & Qt::AltModifier)
      keyInt += Qt::ALT;
  if(modifiers & Qt::MetaModifier)
      keyInt += Qt::META;

  QString seq = QKeySequence(keyInt).toString(QKeySequence::NativeText);
  // qDebug() << "KeySequence:" << seq;

  if (_cb_shortcuts->contains(seq)) {
    try {
      luabind::call_function<void>(_cb_shortcuts->value(seq), _frameNum);
    } catch(const std::exception& ex){
      showLuaException(ex, QString("shortcut '%1' function").arg(seq));
    }

    return; // skip built in command if overridden by shortcut
  }

  switch (e->key()) {

  case Qt::Key_S :
    _simulate = !_simulate;
    emit simulationStateChanged(_simulate);
    break;
  case Qt::Key_P :
    _savePOV = !_savePOV;
    if(_savePOV){
      _firstFrame=_frameNum;
    }
    emit POVStateChanged(_savePOV);
    break;
  case Qt::Key_G :
    _savePNG = !_savePNG;
    emit PNGStateChanged(_savePNG);
    break;
  case Qt::Key_D :
    _deactivation = !_deactivation;
    emit deactivationStateChanged(_deactivation);
    break;
  case Qt::Key_R :
    parse(_scriptContent);
    break;
  case Qt::Key_C :
    resetCamView();
    break;
  default:
    QGLViewer::keyPressEvent(e);
  }
}

void Viewer::addObject(Object *o, int type, int mask) {
  _objects->insert(o);

  if (o->body != NULL) {
    if(!_deactivation){
      o->body->setActivationState(DISABLE_DEACTIVATION);
    }
    dynamicsWorld->addRigidBody(o->body, type, mask);
  }
}

void Viewer::addObjects(QList<Object *> ol, int type, int mask) {
  foreach (Object *o, ol) {
    addObject(o, type, mask);
  }
}

void Viewer::addObjects() {

}

void Viewer::setGravity(btVector3 gravity) {
  dynamicsWorld->setGravity(gravity);
}

btVector3 Viewer::getGravity() {
  return dynamicsWorld->getGravity();
}

void Viewer::setTimeStep(btScalar ts) {
  _timeStep = ts;
}

btScalar Viewer::getTimeStep() {
  return _timeStep;
}

void Viewer::setMaxSubSteps(int mst) {
  _maxSubSteps = mst;
}

int Viewer::getMaxSubSteps() {
  return _maxSubSteps;
}

void Viewer::setFixedTimeStep(btScalar fts) {
  _fixedTimeStep = fts;
}

btScalar Viewer::getFixedTimeStep() {
  return _fixedTimeStep;
}

Viewer::Viewer(QWidget *parent, bool savePNG, bool savePOV) : QGLViewer(parent)  {

  setAttribute(Qt::WA_DeleteOnClose);

  _objects = new QSet<Object *>();
  _constraints = new QSet<btTypedConstraint *>();
  _raycast_vehicles = new QSet<btRaycastVehicle *>();

  L = NULL;

  _savePNG = savePNG; _savePOV = savePOV; setSnapshotFormat("png");
  _simulate = false;
  _deactivation = true;

  collisionCfg = new btDefaultCollisionConfiguration();
  btBroadphaseInterface* broadphase = new btDbvtBroadphase();
  dynamicsWorld = new btDiscreteDynamicsWorld(new btCollisionDispatcher(collisionCfg),
                                              broadphase, new btSequentialImpulseConstraintSolver, collisionCfg);
  btCollisionDispatcher * dispatcher =
          static_cast<btCollisionDispatcher *>(dynamicsWorld ->getDispatcher());
  btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

  _frameNum = 0;
  _firstFrame = 0;

  _cb_shortcuts = new QHash<QString, luabind::object>();

  loadPrefs();

  _cam=NULL;

  // POV-Ray properties
  mPreSDL = "";
  mPostSDL = "";

  startAnimation();
}

void Viewer::close() {
  // qDebug() << "Viewer::close()";
  savePrefs();
  QGLViewer::close();
}

void Viewer::setCamera(Cam *cam) {
  _cam = cam;

  QGLViewer::setCamera( cam );
}

void Viewer::setSavePNG(bool png) {
  _savePNG = png;
}

void Viewer::setSavePOV(bool pov) {
  _savePOV = pov;
}

void Viewer::toggleSavePNG(bool savePNG) {
  _savePNG = savePNG;
}

void Viewer::toggleSavePOV(bool savePOV) {
  _savePOV = savePOV;
}

void Viewer::toggleDeactivation(bool deactivation) {
  _deactivation = deactivation;
}

void Viewer::startSim() {
  _simulate = true;
}

void Viewer::stopSim() {
  _simulate = false;
}

void Viewer::restartSim() {
  parse(_scriptContent);
}

void Viewer::setScriptName(QString sn) {
  _scriptName = sn;
}

void Viewer::emitScriptOutput(const QString& out) {
  emit scriptHasOutput(out);
}

int Viewer::lua_print(lua_State* L) {

  Viewer* p = static_cast<Viewer*>(lua_touserdata(L, lua_upvalueindex(1)));

  if (p) {
    int n = lua_gettop(L);  /* number of arguments */

    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i <= n; i++) {
      const char *s;
      lua_pushvalue(L, -1);  /* function to be called */
      lua_pushvalue(L, i);   /* value to print */
      lua_call(L, 1, 1);
      s = lua_tostring(L, -1);  /* get result */
      if (s == NULL)
        return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
      // if (i>1) p->emitScriptOutput(QString("\t"));
      p->emitScriptOutput(QString(s));
      lua_pop(L, 1);  /* pop result */
    }

    // p->emitScriptOutput(QString("\n"));
  } else {
    return luaL_error(L, "stack has no thread ref", "");
  }

  return 0;
}

bool Viewer::parse(QString txt) {
  emit scriptStopped();

  QMutexLocker locker(&mutex);

  if(_cb_preStop) {
    try {
      luabind::call_function<void>(_cb_preStop, _frameNum);
    } catch(const std::exception& e){
      showLuaException(e, "v:preStop()");
    }
  }

  _parsing = true;
  _has_exception = false;

  _scriptContent = txt;

  bool animStarted = animationIsStarted();

  if (animStarted) {
      stopAnimation();
  }

  emit scriptStarts();

  if (L != NULL) {
      lua_gc(L, LUA_GCCOLLECT, 0); // collect garbage

      clear();

    // invalidate function refs
    _cb_preDraw = luabind::object();
    _cb_postDraw = luabind::object();
    _cb_preSim = luabind::object();
    _cb_postSim = luabind::object();
    _cb_onCommand = luabind::object();

    // lua_close(L);
  } else {
      // setup lua
      L = luaL_newstate();
      // open all standard Lua libs
      luaL_openlibs(L);

      // register all bpp classes
      LuaBullet::luaBind(L);

      Cam::luaBind(L);
      Object::luaBind(L);
      Objects::luaBind(L);
      Cube::luaBind(L);
      Cylinder::luaBind(L);
      Mesh3DS::luaBind(L);
      Palette::luaBind(L);
      Plane::luaBind(L);
      Sphere::luaBind(L);
      Viewer::luaBind(L);

    #ifdef HAS_LUA_QT

    #ifdef HAS_QEXTSERIAL
      QSerialPort::luaBind(L);
    #endif

      // register some qt classes
      register_classes(L);

    #endif

      luaBindInstance(L);

      lua_pushlightuserdata(L, (void*)this);
      lua_pushcclosure(L,  &Viewer::lua_print, 1);
      lua_setglobal(L, "print");
  }

  dynamicsWorld->setGravity(btVector3(0.0f, -G, 0.0f));

  // bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_the_World
  //
  // It's important that timeStep is always less than maxSubSteps*fixedTimeStep,
  // otherwise you are losing time. Mathematically,
  //
  //   timeStep < maxSubSteps * fixedTimeStep
  //
  _timeStep = 0.04;          // roughly 25fps
  //_timeStep = 0.0083;      // roughly 1/120th of a second
  _maxSubSteps = 10;
  _fixedTimeStep = 0.017;    // 1/60th of a second

  int error = luaL_loadstring(L, txt.toAscii().constData())
    || lua_pcall(L, 0, LUA_MULTRET, 0);

  if (error) {
  lua_error = tr("error: %1").arg(lua_tostring(L, -1));

    if (lua_error.contains(QRegExp(tr("stopping$")))) {
      lua_error = tr("script stopped");
      qDebug() << "lua run : script stopped";
    } else {
      // qDebug() << QString("lua run : %1").arg(lua_error);
      emit scriptHasOutput(lua_error);
    }

    lua_pop(L, 1);  /* pop error message from the stack */
  } else {
    lua_error = tr("ok");
  }

  // report_errors(L, error);
  // lua_close(L);

  _frameNum = 0; // reset frames counter
  _firstFrame = 0;

  if (animStarted) {
      startAnimation();
  }

  // qDebug() << "Viewer::parse() end";

  _parsing = false;

  return (error ? false : true);
}

void Viewer::clear() {
  // qDebug() << "Viewer::clear() objects: " << _objects->size();

  // remove all objects
  QSet<Object *>::const_iterator i = _objects->constBegin();
  while (i != _objects->constEnd()) {
    if ((*i)->body != NULL)
      dynamicsWorld->removeRigidBody((*i)->body);
    ++i;
  }

  // remove all contact manifolds
  for (int i = dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--) {
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];

    btRigidBody* body = btRigidBody::upcast(obj);
    if (body && body->getMotionState()) {
      delete body->getMotionState();
    }

    dynamicsWorld->removeCollisionObject( obj );

    delete obj;
  }

  // remove all constraints
  QSet<btTypedConstraint *>::const_iterator it = _constraints->constBegin();
  while (it != _constraints->constEnd()) {
    dynamicsWorld->removeConstraint(*it);
    ++it;
  }

  _objects->clear();

  _constraints->clear();
  _cb_shortcuts->clear();
}

void Viewer::resetCamView() {

    camera()->setPosition(_initialCameraPosition);
    camera()->setOrientation(_initialCameraOrientation);
    updateGL();

}

void Viewer::loadPrefs() {
  QGLViewer::restoreStateFromFile();
}

void Viewer::savePrefs() {
  // qDebug() << "Viewer::savePrefs()";
  QGLViewer::saveStateToFile();
}

void Viewer::openPovFile() {
  QString file;
  QString fileMain;
  QString fileINI;
  QDir sceneDir("export");

  QString sceneName;
  if(!_scriptName.isEmpty()){
    sceneName=_scriptName;
  }else{
    sceneName="no_name.lua";
  }
  sceneDir.mkdir(qPrintable(sceneName));
  file.sprintf("export/%s/%s-%05d.inc", qPrintable(sceneName), qPrintable(sceneName), _frameNum);
  fileMain.sprintf("export/%s/%s.pov", qPrintable(sceneName), qPrintable(sceneName));
  fileINI.sprintf("export/%s/%s.ini", qPrintable(sceneName), qPrintable(sceneName));

  // qDebug() << "saving pov file:" << file;

  _fileINI = new QFile(fileINI);
  _fileINI->open(QFile::WriteOnly | QFile::Truncate);
  QTextStream *ini = new QTextStream(_fileINI);
  *ini << "; Animation INI file generated by Bullet Physics Playground" << endl << endl;
  *ini << "Input_File_Name=" << sceneName << ".pov" << endl;
  *ini << "Output_File_Name=" << sceneName << "-" << endl;
  *ini << "Output_to_File=On" << endl;
  *ini << "Pause_When_Done=Off" << endl;
  *ini << "Verbose=Off" << endl;
  *ini << "Display=On" << endl;
  *ini << "Width=720" << endl;
  *ini << "Height=480" << endl;
  *ini << "+FN" << endl;
  *ini << "+a +j0" << endl;

  // *ini << "Antialias_Threshold=0.3"

  *ini << "+L../../includes" << endl << endl;
  *ini << "Initial_Clock=" << _firstFrame << endl;
  *ini << "Final_Clock=" << _frameNum << endl;
  *ini << "Final_Frame=" << _frameNum << endl;

  *ini << "[LOW]"  << endl <<  "Width=320" << endl <<  "Height=200"  << endl << "Antialias=Off" << endl;
  *ini << "[MED]"  << endl <<  "Width=640" << endl <<  "Height=400"  << endl << "Antialias=Off" << endl;
  *ini << "[HIGH]" << endl << "Width=1280" << endl <<  "Height=800"  << endl
       << "Antialias=On" << endl << "Display=Off" << endl;
  *ini << "[HD]"   << endl << "Width=1920" << endl <<  "Height=1080" << endl
       << "Antialias=On" << endl << "Display=Off" << endl;

  if (_fileINI != NULL) {
    _fileINI->close();
  }

  _fileMain = new QFile(fileMain);
  _fileMain->open(QFile::WriteOnly | QFile::Truncate);

  QTextStream *smain = new QTextStream(_fileMain);
  *smain << "// Main POV file generated by Bullet Physics Playground" << endl << endl;
  *smain << "#include \"settings.inc\"" << endl << endl;
  *smain << "#include concat(concat(\"" << sceneName << "-\",str(clock,-5,0)),\".inc\")" << endl << endl;

  if (_fileMain != NULL) {
    _fileMain->close();
  }

  _file = new QFile(file);
  _file->open(QFile::WriteOnly | QFile::Truncate);

  _stream = new QTextStream(_file);

  *_stream << "// Include file generated by Bullet Physics Playground" << endl << endl;

  Vec pos = camera()->position();

  *_stream << "camera { " << endl;
  *_stream << "  location < " << pos.x << ", " << pos.y << ", " << pos.z << " >" << endl;
  *_stream << "  right -image_width/image_height*x" << endl;

  if (_cam != NULL) {
    btVector3 vLook = _cam->getLookAt();
    *_stream << "  look_at < " << vLook.x() << ", " << vLook.y() << ", " << vLook.z();
    *_stream << "> angle " << _cam->fieldOfView() * 90.0 << endl;
    //// qDebug() << vLook.x() << vLook.y() << vLook.z();
  } else {
    Vec vDir = camera()->viewDirection();
    *_stream << "  look_at < " << pos.x + vDir.x << ", " << pos.y + vDir.y << ", " << pos.z + vDir.z;
    *_stream << "> angle " << camera()->fieldOfView() * 90.0 << endl;
     // qDebug() << pos.x + vDir.x << pos.y + vDir.y << pos.z + vDir.z;
  }
  *_stream << "  }" << endl << endl;
}

void Viewer::closePovFile() {
  if (_file != NULL) {
    _file->close();
  }
}

Viewer::~Viewer() {
  // qDebug() << "Viewer::~Viewer()";
  delete _objects;
  delete dynamicsWorld;
  delete collisionCfg;
}

void Viewer::computeBoundingBox() {
  getAABB(_objects, _aabb);

  btVector3 vmin(_aabb[0], _aabb[1], _aabb[2]);
  btVector3 vmax(_aabb[3], _aabb[4], _aabb[5]);

  float radius = (vmax - vmin).length() * 10.0f;
  // qDebug() << "setSceneRadius() " << radius;
  setSceneRadius(radius);

  btVector3 center = (vmin + vmax) / 2.0f;
  // qDebug() << "setSceneCenter() " << center.z() << center.y() << center.z();
  setSceneCenter(Vec(center.x(), center.y(), 0));

  // setSceneCenter(Vec());
}

void Viewer::init() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);

  computeBoundingBox();

  if (!restoreStateFromFile()) {
    showEntireScene();
  }

  _gl_shininess = btScalar(25.0);
  _gl_specular_col = btVector4(.85f, .85f, .85f, 1.0f);

  _light0 = btVector4(200.0, 200.0, 200.0, 0.2);
  _light1 = btVector4(400.0, 400.0, 200.0, 0.1);

  _gl_ambient = btVector3(.1f, .1f, 1.0f);
  _gl_diffuse = btVector4(.9f, .9f, .9f, 1.0f);
  _gl_specular = btVector4(.85f, .85f, .85f, 1.0f);
  _gl_model_ambient = btVector4(.4f, .4f, 0.4f, 1.0f);

  _initialCameraPosition=camera()->position();
  _initialCameraOrientation=camera()->orientation();
}

void Viewer::draw() {
  if (_parsing) return;

  // Don't know if this is a good idea..
  // computeBoundingBox();

  QMutexLocker locker(&mutex);

  if (L) {
    lua_gc(L, LUA_GCCOLLECT, 0); // collect garbage
    int lsize = lua_gc(L, LUA_GCCOUNT, -1);
    emit statusEvent(QString("LUA_GCCOUNT = %1").arg(lsize));
    lua_gc(L, LUA_GCSTOP, -1);
  }

  glDisable(GL_COLOR_MATERIAL);

  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _gl_shininess);

  GLfloat specular_color[4] = {
      _gl_specular_col.x(),
      _gl_specular_col.y(),
      _gl_specular_col.z(),
      _gl_specular_col.w()
  };

  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  specular_color);

  GLfloat light0_pos[] = {_light0.x(), _light0.y(), _light0.z(), _light0.w()};
  GLfloat light1_pos[] = {_light1.x(), _light1.y(), _light1.z(), _light1.w()};

  GLfloat ambient[]  = { _gl_ambient.x(),  _gl_ambient.y(), _gl_ambient.z() };
  GLfloat diffuse[]  = { _gl_diffuse.x(),  _gl_diffuse.y(), _gl_diffuse.z() };
  GLfloat specular[] = {_gl_specular.x(), _gl_specular.y(), _gl_specular.z() };

  GLfloat lmodel_ambient[] = {
      _gl_model_ambient.x(),
      _gl_model_ambient.y(),
      _gl_model_ambient.z(), _gl_model_ambient.w() };
  GLfloat local_view[] = { 0.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

  glEnable(GL_LIGHTING);

  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

  if (_cb_preDraw) {
    try {
      luabind::call_function<void>(_cb_preDraw, _frameNum);
    } catch(const std::exception& e){
    showLuaException(e, "v:preDraw()");
  }
  }

  if (manipulatedFrame() != NULL) {
    glPushMatrix();
    glMultMatrixd(manipulatedFrame()->matrix());
  }

  if (_savePOV) {
    openPovFile();

    if (!mPreSDL.isEmpty()) {
      *_stream << mPreSDL;
    }
  }

  // qDebug() << "Number of objects:" << _objects->size();
  foreach (Object *o, *_objects) {
    try {
      if (_savePOV) {
        o->render(_stream);
      } else {
        o->render(NULL);
      }
    } catch(const std::exception& e){
      showLuaException(e, "object:render()");
    }
  }

  if (_savePOV) {
    if (!mPostSDL.isEmpty()) {
      *_stream << mPostSDL;
    }
    closePovFile();
  }

  if (manipulatedFrame() != NULL) {
    glPopMatrix();
  }
}

void Viewer::setCBPreDraw(const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_preDraw = fn;
  }
}

void Viewer::setCBPostDraw(const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_postDraw = fn;
  }
}

void Viewer::setCBPreSim(const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_preSim = fn;
  }
}

void Viewer::setCBPostSim(const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_postSim = fn;
  }
}

void Viewer::setCBPreStop(const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_preStop = fn;
  }
}

void Viewer::setCBOnCommand(const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_onCommand = fn;
  }
}

void Viewer::addShortcut(const QString &keys, const luabind::object &fn) {
  if(luabind::type(fn) == LUA_TFUNCTION) {
    _cb_shortcuts->insert(keys, fn);
  }
}

void Viewer::removeShortcut(const QString &keys) {
  _cb_shortcuts->remove(keys);
}

void Viewer::postDraw() {
  QGLViewer::postDraw();

  if(_cb_postDraw) {
    try {
      luabind::call_function<void>(_cb_postDraw, _frameNum);
    } catch(const std::exception& e){
    showLuaException(e, "v:postDraw()");
    }
  }

  // Red dot when EventRecorder is active

  if (animationIsStarted()) {
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(12.0);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POINTS);
    glVertex2i(width()-20, 20);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    stopScreenCoordinatesSystem();
    // restore foregroundColor
    qglColor(foregroundColor());
  }

  if (_simulate) {
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(12.0);
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POINTS);
    glVertex2i(width()-40, 20);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    stopScreenCoordinatesSystem();
    // restore foregroundColor
    qglColor(foregroundColor());
  }

  if (_savePNG) {
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(12.0);
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POINTS);
    glVertex2i(width()-60, 20);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    stopScreenCoordinatesSystem();
    // restore foregroundColor
    qglColor(foregroundColor());
  }

  if (_savePOV) {
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(12.0);
    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    glVertex2i(width()-80, 20);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    stopScreenCoordinatesSystem();
    // restore foregroundColor
    qglColor(foregroundColor());
  }

    if (_deactivation) {
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(12.0);
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_POINTS);
    glVertex2i(width()-100, 20);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    stopScreenCoordinatesSystem();
    // restore foregroundColor
    qglColor(foregroundColor());
  }

}

void Viewer::startAnimation() {
  _time.start();
  QGLViewer::startAnimation();
}

void Viewer::stopAnimation() {
  QGLViewer::stopAnimation();
  //  updateGL();
}

void Viewer::animate() {
  QMutexLocker locker(&mutex);

  if (_has_exception) {
    return;
  }

  // emitScriptOutput(QString("_frameNum = %1").arg(_frameNum));

  // emitScriptOutput("Viewer::animate() begin");

  if (_savePNG) {
    QDir sceneDir("screenshots");
    QString file;
    if(!_scriptName.isEmpty()){
      sceneDir.mkdir(qPrintable(_scriptName));
      file.sprintf("screenshots/%s/%s-%05d.png",
                   qPrintable(_scriptName), qPrintable(_scriptName), _frameNum);
    }else{
      sceneDir.mkdir("no_name");
      file.sprintf("screenshots/no_name/no_name-%05d.png", _frameNum);
    }
    saveSnapshot(file, true);
  }

  if (_simulate) {

    if(_cb_preSim) {
      try {
        luabind::call_function<void>(_cb_preSim, _frameNum);
      } catch(const std::exception& e){
        showLuaException(e, "v:preSim()");
      }
    }

    
    // Find the time elapsed between last time
    // float nbSecsElapsed = 0.08f; // 25 pics/sec
    // float nbSecsElapsed = 1.0 / 24.0;
    // float nbSecsElapsed = _time.elapsed()/10.0f;
    
    // old: dynamicsWorld->stepSimulation(nbSecsElapsed, 10);

    // new: bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_the_World
    dynamicsWorld->stepSimulation(_timeStep, _maxSubSteps, _fixedTimeStep);

    if(_cb_postSim) {
      try {
        luabind::call_function<void>(_cb_postSim, _frameNum);
      } catch(const std::exception& e){
        showLuaException(e, "v:postSim()");
      }
    }

    if (_frameNum > 10)
      emit postDrawShot(_frameNum);

     _frameNum++;
  }

  // Restart the elapsed time counter
  _time.restart();

  // emitScriptOutput("Viewer::animate() end");
}

void Viewer::command(QString cmd) {
  QMutexLocker locker(&mutex);

  // emitScriptOutput("Viewer::command() begin");

  if(_cb_onCommand) {
    try {
      luabind::call_function<void>(_cb_onCommand, cmd);
    } catch(const std::exception& e){
      showLuaException(e, "v:onCommand()");
    }
  }

  // emitScriptOutput("Viewer::command() end");
}

void Viewer::showLuaException(const std::exception &e, const QString& context) {
  _has_exception = true;

  if ( std::string const *stack = boost::get_error_info<stack_info>(e) ) {
    std::cout << stack << std::endl;
  }

  // the error message should be on top of the stack
  QString luaWhat = QString("%1").arg(lua_tostring(L, -1));

  emitScriptOutput(QString("%1 in %2: %3").arg(e.what()).arg(context).arg(luaWhat));
}

void Viewer::setGLShininess(const btScalar &s) {
  _gl_shininess = s;
}

btScalar Viewer::getGLShininess() const {
  return _gl_shininess;
}

void Viewer::setGLSpecularColor(const btVector4 &col) {
  _gl_specular_col = col;
}

btVector4 Viewer::getGLSpecularColor() const {
  return _gl_specular_col;
}

void Viewer::setGLSpecularCol(const btScalar col) {
  _gl_specular_col = btVector4(col, col, col, col);
}

btScalar Viewer::getGLSpecularCol() const {
  return _gl_specular_col.length();
}

void Viewer::setGLLight0(const btVector4 &pos) {
    _light0 = pos;
}

btVector4 Viewer::getGLLight0() const {
    return _light0;
}

void Viewer::setGLLight1(const btVector4 &pos) {
    _light1 = pos;
}

btVector4 Viewer::getGLLight1() const {
    return _light1;
}

// Vector

void Viewer::setGLAmbient(const btVector3 &am) {
    _gl_ambient = am;
}

btVector3 Viewer::getGLAmbient() const {
    return _gl_ambient;
}

void Viewer::setGLDiffuse(const btVector4 &col) {
  _gl_diffuse = col;
}

btVector4 Viewer::getGLDiffuse() const {
  return _gl_diffuse;
}

void Viewer::setGLSpecular(const btVector4 &col) {
  _gl_specular = col;
}

btVector4 Viewer::getGLSpecular() const {
  return _gl_specular;
}

void Viewer::setGLModelAmbient(const btVector4 &am) {
    _gl_model_ambient = am;
}

btVector4 Viewer::getGLModelAmbient() const {
    return _gl_model_ambient;
}

// Percent

void Viewer::setGLAmbientPercent(const btScalar am) {
    _gl_ambient = btVector3(am, am, am);
}

btScalar Viewer::getGLAmbientPercent() const {
    return _gl_ambient.length();
}

void Viewer::setGLDiffusePercent(const btScalar col) {
  _gl_diffuse = btVector4(col, col, col, 1);
}

btScalar Viewer::getGLDiffusePercent() const {
  return _gl_diffuse.length();
}

void Viewer::setGLSpecularPercent(const btScalar col) {
  _gl_specular = btVector4(col, col, col, 1);
}

btScalar Viewer::getGLSpecularPercent() const {
  return _gl_specular.length();
}

void Viewer::setGLModelAmbientPercent(const btScalar am) {
    _gl_model_ambient = btVector4(am, am, am, 1);
}

btScalar Viewer::getGLModelAmbientPercent() const {
    return _gl_model_ambient.length();
}

// POV-Ray properties

void Viewer::setPreSDL(const QString &preSDL) {
    mPreSDL = preSDL;
}

QString Viewer::getPreSDL() const {
    return mPreSDL;
}

void Viewer::setPostSDL(const QString &postSDL) {
    mPostSDL = postSDL;
}

QString Viewer::getPostSDL() const {
    return mPostSDL;
}

void Viewer::setPrefs(QString key, QString value) {
    _settings->beginGroup("lua");
    _settings->setValue(key, value);
    _settings->endGroup();
}

QString Viewer::getPrefs(QString key, QString defaultValue) const {
    _settings->beginGroup("lua");
    QString v =_settings->value(key, defaultValue).toString();
    _settings->endGroup();
    return v;
}

void Viewer::setSettings(QSettings *settings) {
    _settings = settings;
}
