/***************************************************************************
 * pythonscript.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ***************************************************************************/

#include "pythonscript.h"
#include "pythonmodule.h"
#include "pythoninterpreter.h"
//#include "../api/object.h"
//#include "../api/list.h"
//#include "../main/manager.h"
#include "../main/scriptcontainer.h"
//#include "../api/qtobject.h"
//#include "../api/interpreter.h"

#include <kdebug.h>

using namespace Kross::Python;

namespace Kross { namespace Python {

    /// @internal
    class PythonScriptPrivate
    {
        public:

            /**
            * The \a Py::Module instance this \a PythonScript
            * has as local context.
            */
            Py::Module* m_module;

            /**
            * The PyCodeObject object representing the
            * compiled python code. Internaly we first
            * compile the python code and later execute
            * it.
            */
            Py::Object* m_code;

            /**
            * A list of functionnames.
            */
            QStringList m_functions;

            /**
            * A list of classnames.
            */
            QStringList m_classes;

            //QMap<QString, Kross::Api::Object::Ptr> m_functions;
            //QMap<QString, Kross::Api::Object::Ptr> m_classes;
            //QValueList<Kross::Api::Object::Ptr> m_classinstances;
    };

}}

PythonScript::PythonScript(Kross::Api::Interpreter* interpreter, Kross::Api::ScriptContainer* scriptcontainer)
    : Kross::Api::Script(interpreter, scriptcontainer)
    , d(new PythonScriptPrivate())
{
#ifdef KROSS_PYTHON_SCRIPT_CTOR_DEBUG
    kdDebug() << "PythonScript::PythonScript() Constructor." << endl;
#endif
    d->m_module = 0;
    d->m_code = 0;
}

PythonScript::~PythonScript()
{
#ifdef KROSS_PYTHON_SCRIPT_DTOR_DEBUG
    kdDebug() << "PythonScript::~PythonScript() Destructor." << endl;
#endif
    finalize();
    delete d;
}

void PythonScript::initialize()
{
    finalize();
    clearException(); // clear previously thrown exceptions.

    try {
        PyObject* pymod = PyModule_New((char*)m_scriptcontainer->getName().latin1());
        d->m_module = new Py::Module(pymod, true);
        if(! d->m_module)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Failed to initialize local module context for script '%1'").arg( m_scriptcontainer->getName() )) );

#ifdef KROSS_PYTHON_SCRIPT_INIT_DEBUG
        kdDebug() << QString("PythonScript::initialize() module='%1' refcount='%2'").arg(d->m_module->as_string().c_str()).arg(d->m_module->reference_count()) << endl;
#endif

        // Set the "self" variable to point to the ScriptContainer
        // we are using for the script. That way we are able to
        // simply access the ScriptContainer itself from within
        // python scripting code.
        Py::Dict moduledict = d->m_module->getDict();
        moduledict["self"] = PythonExtension::toPyObject( m_scriptcontainer );
        //moduledict["parent"] = PythonExtension::toPyObject( m_manager );



// Prepare local context.
QString s =
    "import sys\n"

    "class Redirect:\n"
    "  def __init__(self, target):\n"
    "    self.target = target\n"
    "  def write(self, s):\n"
    "    self.target.call(s)\n"

    "if self.has(\"stdout\"):\n"
    "  sys.stdout = Redirect( self.get(\"stdout\") )\n"
    "if self.has(\"stderr\"):\n"
    "  sys.stderr = Redirect( self.get(\"stderr\") )\n"

    "import cStringIO\n"
    "sys.stdin = cStringIO.StringIO()\n"
    ;
Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->m_mainmodule->getDict();
PyObject* pyrun = PyRun_String((char*)s.latin1(), Py_file_input, mainmoduledict.ptr(), moduledict.ptr());
if(! pyrun)
    throw Py::Exception(); // throw exception
Py_XDECREF(pyrun); // free the reference.




        // Compile the python script code. It will be later on request
        // executed. That way we cache the compiled code.
        PyObject* code = 0;
        bool restricted = ((PythonInterpreter*)m_interpreter)->getOption("restricted", QVariant((bool)true)).toBool();
        if(restricted) {
            // Use the RestrictedPython module wrapped by the PythonSecurity class.
/*TODO
            code = m_interpreter->m_security->compile_restricted(
                m_scriptcontainer->getCode(),
                m_scriptcontainer->getName(),
                "exec"
            );
*/
        }
        else {
            // Just compile the code without any restrictions.
            code = Py_CompileString(
                (char*)m_scriptcontainer->getCode().latin1(),
                (char*)m_scriptcontainer->getName().latin1(),
                Py_file_input
            );
        }

        if(! code)
            throw Py::Exception();
        d->m_code = new Py::Object(code, true);
    }
    catch(Py::Exception& e) {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( QString("Failed to compile python code: %1").arg(Py::value(e).as_string().c_str()) ) );
    }
}

void PythonScript::finalize()
{
#ifdef KROSS_PYTHON_SCRIPT_FINALIZE_DEBUG
    if(d->m_module)
        kdDebug() << QString("PythonScript::finalize() module='%1' refcount='%2'").arg(d->m_module->as_string().c_str()).arg(d->m_module->reference_count()) << endl;
#endif

    delete d->m_module; d->m_module = 0;
    delete d->m_code; d->m_code = 0;
    d->m_functions.clear();
    d->m_classes.clear();
}

const QStringList& PythonScript::getFunctionNames()
{
    if(! d->m_module)
        initialize(); //TODO catch exception
    return d->m_functions;
    /*
    QStringList list;
    Py::List l = d->m_module->getDict().keys();
    int length = l.length();
    for(Py::List::size_type i = 0; i < length; ++i)
        list.append( l[i].str().as_string().c_str() );
    return list;
    */
}

Kross::Api::Object::Ptr PythonScript::execute()
{
#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
    kdDebug() << QString("PythonScript::execute()") << endl;
#endif

    try {
        if(! d->m_module)
            initialize();

        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->m_mainmodule->getDict();
        PyObject* pyresult = PyEval_EvalCode(
            (PyCodeObject*)d->m_code->ptr(),
            mainmoduledict.ptr(),
            d->m_module->getDict().ptr()
        );
        if(! pyresult)
            throw Py::Exception();
        Py::Object result(pyresult, true);

#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
        kdDebug()<<"PythonScript::execute() result="<<result.as_string().c_str()<<endl;
#endif

        Py::Dict moduledict( d->m_module->getDict().ptr() );
        for(Py::Dict::iterator it = moduledict.begin(); it != moduledict.end(); ++it) {
            Py::Dict::value_type vt(*it);
            if(PyClass_Check( vt.second.ptr() )) {
#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
                kdDebug() << QString("PythonScript::execute() class '%1' added.").arg(vt.first.as_string().c_str()) << endl;
#endif
                d->m_classes.append( vt.first.as_string().c_str() );
            }
            else if(vt.second.isCallable()) {
#ifdef KROSS_PYTHON_SCRIPT_EXEC_DEBUG
                kdDebug() << QString("PythonScript::execute() function '%1' added.").arg(vt.first.as_string().c_str()) << endl;
#endif
                d->m_functions.append( vt.first.as_string().c_str() );
            }
        }

        Kross::Api::Object::Ptr r = PythonExtension::toObject(result);
        return r;
    }
    catch(Py::Exception& e) {
        setException( new Kross::Api::Exception(QString("Failed to execute python code: %1").arg(Py::value(e).as_string().c_str())) );
    }
    catch(Kross::Api::Exception::Ptr e) {
        setException(e);
    }

    return 0; // return nothing if exception got thrown.

/*
    if(! d->m_module) initialize();
    try {
        Py::Dict mainmoduledict = ((PythonInterpreter*)m_interpreter)->m_module->getDict();
        PyObject* pyrun = PyRun_String(
            (char*)m_scriptcontainer->getCode().latin1(),
            Py_file_input,
            mainmoduledict.ptr(),
            d->m_module->getDict().ptr()
        );
        if(! pyrun) {
            Py::Object errobj = Py::value(Py::Exception()); // get last error
            throw Kross::Api::RuntimeException(QString("Python Exception: %1").arg(errobj.as_string().c_str()));
        }
        Py::Object run(pyrun, true); // the run-object takes care of freeing our pyrun pyobject.
        //kdDebug() << QString("PythonScript::execute --------------------------- 1") << endl;
        Py::Dict moduledict( d->m_module->getDict().ptr() );
        for(Py::Dict::iterator it = moduledict.begin(); it != moduledict.end(); ++it) {
            Py::Dict::value_type vt(*it);
            if(PyClass_Check( vt.second.ptr() )) {
                kdDebug() << QString("PythonScript::execute() class '%1' added.").arg(vt.first.as_string().c_str()) << endl;
                d->m_classes.append( vt.first.as_string().c_str() );
            }
            else if(vt.second.isCallable()) {
                kdDebug() << QString("PythonScript::execute() function '%1' added.").arg(vt.first.as_string().c_str()) << endl;
                d->m_functions.append( vt.first.as_string().c_str() );
            }
*/
            /*
            QString s;
            if(vt.second.isCallable()) s += "isCallable ";
            if(vt.second.isDict()) s += "isDict ";
            if(vt.second.isList()) s += "isList ";
            if(vt.second.isMapping()) s += "isMapping ";
            if(vt.second.isNumeric()) s += "isNumeric ";
            if(vt.second.isSequence()) s += "isSequence ";
            if(vt.second.isTrue()) s += "isTrue ";
            if(vt.second.isInstance()) s += "isInstance ";
            if(PyClass_Check( vt.second.ptr() )) s += "vt.second.isClass ";
            Py::String rf = vt.first.repr();
            Py::String rs = vt.second.repr();
            kdDebug() << QString("PythonScript::execute d->m_module->getDict() rs='%1' vt.first='%2' rf='%3' test='%4' s='%5'")
                        .arg(rs.as_string().c_str())
                        .arg(vt.first.as_string().c_str())
                        .arg(rf.as_string().c_str())
                        .arg("")
                        .arg(s) << endl;
            //m_objects.replace(vt.first.repr().as_string().c_str(), vt.second);
            if(PyClass_Check( vt.second.ptr() )) {
                //PyObject *aclarg = Py_BuildValue("(s)", rs.as_string().c_str());
                PyObject *pyinst = PyInstance_New(vt.second.ptr(), 0, 0);//aclarg, 0);
                //Py_DECREF(aclarg);
                if (pyinst == 0)
                    kdDebug() << QString("PythonScript::execute PyInstance_New() returned NULL !!!") << endl;
                else {
                    Py::Object inst(pyinst, true);
                    kdDebug() << QString("====> PythonScript::execute inst='%1'").arg(inst.as_string().c_str()) << endl;
                }
            }
            */
}

Kross::Api::Object::Ptr PythonScript::callFunction(const QString& name, Kross::Api::List::Ptr args)
{
#ifdef KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
    kdDebug() << QString("PythonScript::callFunction(%1, %2)")
                 .arg(name)
                 .arg(args ? QString::number(args->count()) : QString("NULL"))
                 << endl;
#endif

    if(hadException()) return 0; // abort if we had an unresolved exception.

    if(! d->m_module) {
        setException( new Kross::Api::Exception(QString("Script not initialized.")) );
        return 0;
    }

    try {
        Py::Dict moduledict = d->m_module->getDict();

        // Try to determinate the function we like to execute.
        PyObject* func = PyDict_GetItemString(moduledict.ptr(), name.latin1());

        if( (! d->m_functions.contains(name)) || (! func) )
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("No such function '%1'.").arg(name)) );

        Py::Callable funcobject(func, true); // the funcobject takes care of freeing our func pyobject.

        // Check if the object is really a function and therefore callable.
        if(! funcobject.isCallable())
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Function is not callable.")) );

        // Call the function.
        Py::Object result = funcobject.apply(PythonExtension::toPyTuple(args));

/* TESTCASE
//PyObject* pDict = PyObject_GetAttrString(func,"__dict__");
//kdDebug()<<"INT => "<< PyObject_IsInstance(func,d->m_module->ptr()) <<endl;
//( PyCallable_Check(func)){//PyModule_CheckExact(func)){//PyModule_Check(func)){//PyMethod_Check(func)){//PyMethod_Function(func)){//PyInstance_Check(func) )
        kdDebug() << QString("PythonScript::callFunction result='%1' type='%2'").arg(result.as_string().c_str()).arg(result.type().as_string().c_str()) << endl;
        if(result.isInstance()) {
            kdDebug() << ">>> IS_INSTANCE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
        }
        if(result.isCallable()) {
            kdDebug() << ">>> IS_CALLABLE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
        }
*/

        return PythonExtension::toObject(result);
    }
    catch(Py::Exception& e) {
        Py::Object errobj = Py::value(e);
        setException( new Kross::Api::Exception(QString("Python Exception: %1").arg(errobj.as_string().c_str())) );
    }
    catch(Kross::Api::Exception::Ptr e) {
        setException(e);
    }

    return 0; // return nothing if exception got thrown.
}

const QStringList& PythonScript::getClassNames()
{
    if(! d->m_module)
        initialize(); //TODO catch exception
    return d->m_classes;
}

Kross::Api::Object::Ptr PythonScript::classInstance(const QString& name)
{
    if(hadException()) return 0; // abort if we had an unresolved exception.

    if(! d->m_module) {
        setException( new Kross::Api::Exception(QString("Script not initialized.")) );
        return 0;
    }

    try {
        Py::Dict moduledict = d->m_module->getDict();

        // Try to determinate the class.
        PyObject* pyclass = PyDict_GetItemString(moduledict.ptr(), name.latin1());
        if( (! d->m_classes.contains(name)) || (! pyclass) )
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("No such class '%1'.").arg(name)) );

        //PyClass_Check( vt.second.ptr() ))
        //PyObject *aclarg = Py_BuildValue("(s)", rs.as_string().c_str());
        PyObject *pyobj = PyInstance_New(pyclass, 0, 0);//aclarg, 0);
        if(! pyobj)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(QString("Failed to create instance of class '%1'.").arg(name)) );

        Py::Object classobject(pyobj, true);

#ifdef KROSS_PYTHON_SCRIPT_CLASSINSTANCE_DEBUG
        kdDebug() << QString("PythonScript::classInstance() inst='%1'").arg(classobject.as_string().c_str()) << endl;
#endif
        return PythonExtension::toObject(classobject);
    }
    catch(Py::Exception& e) {
        Py::Object errobj = Py::value(e);
        setException( Kross::Api::Exception::Ptr( new Kross::Api::Exception(errobj.as_string().c_str()) ) );
    }
    catch(Kross::Api::Exception::Ptr e) {
        setException(e);
    }

    return 0; // return nothing if exception got thrown.
}

