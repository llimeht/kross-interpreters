/***************************************************************************
 * pythoninterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "pythoninterpreter.h"
#include "pythonscript.h"
#include "pythonmodule.h"
#include "pythonvariant.h"

//#include <kglobal.h>
//#include <kstandarddirs.h>

#if defined(Q_WS_WIN)
  #define PYPATHDELIMITER ";"
#else
  #define PYPATHDELIMITER ":"
#endif

// The in krossconfig.h defined KROSS_EXPORT_INTERPRETER macro defines an
// exported C function used as factory for Kross::PythonInterpreter instances.
KROSS_EXPORT_INTERPRETER( Kross::PythonInterpreter )

using namespace Kross;

namespace Kross {

    /// \internal
    class PythonInterpreterPrivate
    {
        public:
            /// The __main__ python module.
            PythonModule* mainmodule;

            PythonInterpreterPrivate() : mainmodule(0) {}
    };

}

PythonInterpreter::PythonInterpreter(Kross::InterpreterInfo* info)
    : Kross::Interpreter(info)
    , d(new PythonInterpreterPrivate())
{
    // Initialize the python interpreter.
    initialize();

    // Set name of the program.
    Py_SetProgramName(const_cast<char*>("Kross"));

    /*
    // Set arguments.
    //char* comm[0];
    const char* comm = const_cast<char*>("kross"); // name.
    PySys_SetArgv(1, comm);
    */

    // In the python sys.path are all module-directories are
    // listed in.
    QString path;

    // First import the sys-module to remember it's sys.path
    // list in our path QString.
    Py::Module sysmod( PyImport_ImportModule( (char*)"sys" ), true );
    Py::Dict sysmoddict = sysmod.getDict();
    Py::Object syspath = sysmoddict.getItem("path");
    if(syspath.isList()) {
        Py::List syspathlist = syspath;
        for(Py::List::iterator it = syspathlist.begin(); it != syspathlist.end(); ++it) {
            if( ! (*it).isString() ) continue;
            QString s = PythonType<QString>::toVariant(*it);
            path.append( s + PYPATHDELIMITER );
        }
    }
    else
        path = Py_GetPath();

#if 0
    // Determinate additional module-paths we like to add.
    // First add the global Kross modules-path.
    QStringList krossdirs = KGlobal::dirs()->findDirs("data", "kross/python");
    for(QStringList::Iterator krossit = krossdirs.begin(); krossit != krossdirs.end(); ++krossit)
        path.append(*krossit + PYPATHDELIMITER);
    // Then add the application modules-path.
    QStringList appdirs = KGlobal::dirs()->findDirs("appdata", "kross/python");
    for(QStringList::Iterator appit = appdirs.begin(); appit != appdirs.end(); ++appit)
        path.append(*appit + PYPATHDELIMITER);
#endif

    // Set the extended sys.path.
    PySys_SetPath( (char*) path.toLatin1().data() );

    #ifdef KROSS_PYTHON_INTERPRETER_DEBUG
        krossdebug(QString("Python ProgramName: %1").arg(Py_GetProgramName()));
        krossdebug(QString("Python ProgramFullPath: %1").arg(Py_GetProgramFullPath()));
        krossdebug(QString("Python Version: %1").arg(Py_GetVersion()));
        krossdebug(QString("Python Platform: %1").arg(Py_GetPlatform()));
        krossdebug(QString("Python Prefix: %1").arg(Py_GetPrefix()));
        krossdebug(QString("Python ExecPrefix: %1").arg(Py_GetExecPrefix()));
        //krossdebug(QString("Python Path: %1").arg(Py_GetPath()));
        //krossdebug(QString("Python System Path: %1").arg(path));
    #endif

    // Initialize the main module.
    d->mainmodule = new PythonModule(this);

    // The main dictonary.
    Py::Dict moduledict = d->mainmodule->getDict();
    //TODO moduledict["KrossPythonVersion"] = Py::Int(KROSS_PYTHON_VERSION);

    // Prepare the interpreter.
    QString s =
        //"# -*- coding: iso-8859-1 -*-\n"
        //"import locale\n"
        //"locale.setlocale(locale.LC_ALL, '')\n"
        //"# -*- coding: latin-1 -*\n"
        //"# -*- coding: utf-8 -*-\n"
        //"import locale\n"
        //"locale.setlocale(locale.LC_ALL, '')\n"
        //"from __future__ import absolute_import\n"
        "import sys\n"
        //"import os, os.path\n"
        //"sys.setdefaultencoding('latin-1')\n"

        // Dirty hack to get sys.argv defined. Needed for e.g. TKinter.
        "sys.argv = ['']\n"

        // On the try to read something from stdin always return an empty
        // string. That way such reads don't block our script.
        // Deactivated since sys.stdin has the encoding attribute needed
        // by e.g. LiquidWeather and those attr is missing in StringIO
        // and cause it's buildin we can't just add it but would need to
        // implement our own class. Grrrr, what a stupid design :-/
        //"try:\n"
        //"    import cStringIO\n"
        //"    sys.stdin = cStringIO.StringIO()\n"
        //"except:\n"
        //"    pass\n"

        // Class to redirect something. We use this class e.g. to redirect
        // <stdout> and <stderr> to a c++ event.
        //"class Redirect:\n"
        //"  def __init__(self, target):\n"
        //"    self.target = target\n"
        //"  def write(self, s):\n"
        //"    self.target.call(s)\n"

        // Wrap builtin __import__ method. All import requests are
        // first redirected to our PythonModule.import method and
        // if the call returns None, then we call the original
        // python import mechanism.
        "import __builtin__\n"
        "import __main__\n"
        "import traceback\n"
        "sys.modules['_oldmain'] = sys.modules['__main__']\n"
        "_main_builtin_import_ = __main__.__builtin__.__import__\n"
        "class _Importer:\n"
        "   def __init__(self, script):\n"
        "       self.script = script\n"
        "       self.realImporter = __main__.__builtin__.__import__\n"
        "       __main__.__builtin__.__import__ = self._import\n"
        "   def _import(self, name, globals=None, locals=None, fromlist=[], level = -1):\n"
        //"       try:\n"
        //"           print \"1===========> _Importer name=%s fromlist=%s\" % (name,fromlist)\n"
#if PY_MAJOR_VERSION >= 3 || (PY_MAJOR_VERSION >= 2 && PY_MINOR_VERSION >= 5)
        "           mod = __main__._import(self.script, name, globals, locals, fromlist, level)\n"
#else
        "           mod = __main__._import(self.script, name, globals, locals, fromlist)\n"
#endif
        "           if mod == None:\n"
        "               if name == 'qt':\n"
        "                   raise ImportError('Import of the PyQt3 module is not allowed. Please use PyQt4 instead.')\n"
        "               if name == 'dcop':\n"
        "                   raise ImportError('Import of the KDE3 DCOP module is not allowed. Please use PyQt4 DBUS instead.')\n"
#if PY_MAJOR_VERSION >= 3 || (PY_MAJOR_VERSION >= 2 && PY_MINOR_VERSION >= 5)
        "               mod = self.realImporter(name, globals, locals, fromlist, level)\n"
#else
        "               mod = self.realImporter(name, globals, locals, fromlist)\n"
#endif
        "           if mod != None:\n"
        //"               print \"3===========> _Importer name=%s fromlist=%s\" % (name,fromlist)\n"
        "               if globals != None and (not fromlist or len(fromlist)==0 or '*' in fromlist):\n"
        "                   globals[name] = mod\n"
        "           return mod\n"
        //"       except ImportError:\n"
        //"       except:\n"
        //"           print \"9===========> _Importer Trying ImportError with name=%s fromlist=%s insysmodules=%s\" % (name,fromlist,name in sys.modules)\n"
        //"           print \" \".join( traceback.format_exception(sys.exc_info()[0],sys.exc_info()[1],sys.exc_info()[2]) )\n"
        //"       return None\n"

/*
        "       print \"_Importer name=%s fromlist=%s\" % (name,fromlist)\n"
        "       if fromlist == None:\n"
        "           mod = __main__._import(self.script, name, globals, locals, fromlist)\n"
        "           if mod != None:\n"
        "               print \"2===========> _Importer name=%s fromlist=%s\" % (name,fromlist)\n"
        "               globals[name] = mod\n"
        "               return mod\n"
        //"           if name in sys.modules:\n" // hack to preserve module paths, needed e.g. for "import os.path"
        //"               print \"3===========> _Importer name=%s fromlist=%s\" % (name,fromlist)\n"
        //"               return sys.modules[name]\n"
        "       print \"3===========> _Importer Trying realImporter with name=%s fromlist=%s insysmodules=%s\" % (name,fromlist,name in sys.modules)\n"
        "       try:\n"
        "           mod = self.realImporter(name, globals, locals, fromlist)\n"
        "           print \"4===========> _Importer Trying realImporter with name=%s fromlist=%s insysmodules=%s module=%s\" % (name,fromlist,name in sys.modules,mod)\n"
        //"           mod.__init__(name)\n"
        //"           globals[name] = mod\n"
        //"           sys.modules[name] = mod\n"
        "           print \"5===========> _Importer Trying realImporter with name=%s fromlist=%s insysmodules=%s module=%s\" % (name,fromlist,name in sys.modules,mod)\n"
        "           return mod\n"
        "       except ImportError:\n"
        "           print \"6===========> _Importer Trying ImportError with name=%s fromlist=%s insysmodules=%s\" % (name,fromlist,name in sys.modules)\n"
        "           n = name.split('.')\n"
        "           if len(n) >= 2:\n"
        "               print \"7===========> _Importer Trying ImportError with name=%s fromlist=%s insysmodules=%s\" % (name,fromlist,name in sys.modules)\n"
        "               m = self._import(\".\".join(n[:-1]),globals,locals,[n[-1],])\n"
        "               print \"8===========> _Importer Trying ImportError with name=%s fromlist=%s insysmodules=%s\" % (name,fromlist,name in sys.modules)\n"
        "               return self.realImporter(name, globals, locals, fromlist)\n"
        "           print \"9===========> _Importer Trying ImportError with name=%s fromlist=%s insysmodules=%s\" % (name,fromlist,name in sys.modules)\n"
        "           raise\n"
*/
        ;

    PyObject* pyrun = PyRun_String(s.toLatin1().data(), Py_file_input, moduledict.ptr(), moduledict.ptr());
    if(! pyrun) {
        Py::Object errobj = Py::value(Py::Exception()); // get last error
        setError( QString("Failed to prepare the __main__ module: %1").arg(errobj.as_string().c_str()) );
    }
    Py_XDECREF(pyrun); // free the reference.
}

PythonInterpreter::~PythonInterpreter()
{
    // Free the main module.
    delete d->mainmodule; d->mainmodule = 0;
    // Finalize the python interpreter.
    finalize();
    // Delete the private d-pointer.
    delete d;
}

void PythonInterpreter::initialize()
{
    // Initialize python.
    Py_Initialize();

    /* Not needed cause we use the >= Python 2.3 GIL-mechanism.
    PyThreadState* d->globalthreadstate, d->threadstate;
    // First we have to initialize threading if python supports it.
    PyEval_InitThreads();
    // The main thread. We don't use it later.
    d->globalthreadstate = PyThreadState_Swap(NULL);
    d->globalthreadstate = PyEval_SaveThread();
    // We use an own sub-interpreter for each thread.
    d->threadstate = Py_NewInterpreter();
    // Note that this application has multiple threads.
    // It maintains a separate interp (sub-interpreter) for each thread.
    PyThreadState_Swap(d->threadstate);
    // Work done, release the lock.
    PyEval_ReleaseLock();
    */

    // Initialize our stuff.
    PythonExtension::init_type();
}

void PythonInterpreter::finalize()
{
    /* Not needed cause we use the >= Python 2.3 GIL-mechanism.
    // Lock threads.
    PyEval_AcquireLock();
    // Free the used thread.
    PyEval_ReleaseThread(d->threadstate);
    // Set back to rememberd main thread.
    PyThreadState_Swap(d->globalthreadstate);
    // Work done, unlock.
    PyEval_ReleaseLock();
    */

    // Finalize python.
    Py_Finalize();
}

Kross::Script* PythonInterpreter::createScript(Kross::Action* Action)
{
    //if(hadError()) return 0;
    return new PythonScript(this, Action);
}


PythonModule* PythonInterpreter::mainModule() const
{
    return d->mainmodule;
}

void PythonInterpreter::extractException(QStringList& errorlist, int& lineno)
{
    lineno = -1;

    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    Py_FlushLine();
    PyErr_NormalizeException(&type, &value, &traceback);
    if(traceback) {
        Py::List tblist;
        try {
            Py::Module tbmodule( PyImport_Import(Py::String("traceback").ptr()), true );
            Py::Dict tbdict = tbmodule.getDict();
            Py::Callable tbfunc(tbdict.getItem("format_tb"));
            Py::Tuple args(1);
            args.setItem(0, Py::Object(traceback));
            tblist = tbfunc.apply(args);
            uint length = tblist.length();
            for(Py::List::size_type i = 0; i < length; ++i)
                errorlist.append( Py::Object(tblist[i]).as_string().c_str() );
        }
        catch(Py::Exception& e) {
            QString err = Py::value(e).as_string().c_str();
            e.clear(); // exception is handled. clear it now.
            #ifdef KROSS_PYTHON_EXCEPTION_DEBUG
                krosswarning( QString("Kross::PythonScript::toException() Failed to fetch a traceback: %1").arg(err) );
            #endif
        }

        PyObject *next;
        while (traceback && traceback != Py_None) {
            PyFrameObject *frame = (PyFrameObject*)PyObject_GetAttrString(traceback, const_cast< char* >("tb_frame"));
            {
                PyObject *getobj = PyObject_GetAttrString(traceback, const_cast< char* >("tb_lineno") );
                lineno = PyInt_AsLong(getobj);
                Py_DECREF(getobj);
            }
            if(Py_OptimizeFlag) {
                PyObject *getobj = PyObject_GetAttrString(traceback, const_cast< char* >("tb_lasti") );
                int lasti = PyInt_AsLong(getobj);
                Py_DECREF(getobj);
                lineno = PyCode_Addr2Line(frame->f_code, lasti);
            }

            //const char* filename = PyString_AsString(frame->f_code->co_filename);
            //const char* name = PyString_AsString(frame->f_code->co_name);
            //errorlist.append( QString("%1#%2: \"%3\"").arg(filename).arg(lineno).arg(name) );

            //Py_DECREF(frame); // don't free cause we don't own it.

            next = PyObject_GetAttrString(traceback, const_cast< char* >("tb_next") );
            Py_DECREF(traceback);
            traceback = next;
        }
    }

    if(lineno < 0 && value && PyObject_HasAttrString(value, const_cast< char* >("lineno"))) {
        PyObject *getobj = PyObject_GetAttrString(value, const_cast< char* >("lineno") );
        if(getobj) {
            lineno = PyInt_AsLong(getobj);
            Py_DECREF(getobj);
        }
    }

    #ifdef KROSS_PYTHON_EXCEPTION_DEBUG
        //krossdebug( QString("PythonInterpreter::extractException: %1").arg( Py::Object(value).as_string().c_str() ) );
        krossdebug( QString("PythonInterpreter::extractException:\n%1").arg( errorlist.join("\n") ) );
    #endif
    PyErr_Restore(type, value, traceback);
}
