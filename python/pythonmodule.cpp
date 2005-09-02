/***************************************************************************
 * pythonmodule.cpp
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

#include "pythonmodule.h"
#include "pythoninterpreter.h"

#include <kdebug.h>

using namespace Kross::Python;

namespace Kross { namespace Python {

    /// @internal
    class PythonModulePrivate
    {
        public:

            /**
             * The \a PythonInterpreter instance this module is
             * part of.
             */
            PythonInterpreter* m_interpreter;

            /**
             * List of \a PythonExtension instances accessible
             * via this \a PythonModule instance.
             */
            QMap<QString, PythonExtension*> m_modules;

    };

}}

PythonModule::PythonModule(PythonInterpreter* interpreter)
    : Py::ExtensionModule<PythonModule>("__main__")
    , d(new PythonModulePrivate())
{
#ifdef KROSS_PYTHON_MODULE_DEBUG
    kdDebug() << QString("Kross::Python::PythonModule::Constructor") << endl;
#endif

    d->m_interpreter = interpreter;

    //add_varargs_method("get", &PythonModule::get, "FIXME: Documentation");
    add_varargs_method("_import", &PythonModule::import, "FIXME: Documentation");

    initialize("FIXME: Documentation"); //TODO initialize( object->getDescription().latin1() );

/*
    kdDebug()<<"$$$$$$$$$$$$---------------------------------------------------"<<endl;
    Py::List l = module().dir();
    for(Py::List::size_type i=0; i < l.length(); ++i) {
            kdDebug() << QString("-------------------") << endl;
            kdDebug() << QString("dir() = %1").arg( l[i].str().as_string().c_str() ) << endl;
            kdDebug() << QString("dir().dir() = %1").arg( Py::Object(l[i]).dir().as_string().c_str() ) << endl;
    }
    kdDebug()<<"$$$$$$$$$$$$---------------------------------------------------"<<endl;
*/

/*
kdDebug() << QString("Kross::Python::PythonModule::Constructor 1") << endl;
Py::Dict moduledict = moduleDictionary();
moduledict["self"] = Py::String("test!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
kdDebug() << QString("Kross::Python::PythonModule::Constructor 2") << endl;
*/
}

PythonModule::~PythonModule()
{
#ifdef KROSS_PYTHON_MODULE_DEBUG
    kdDebug() << QString("Kross::Python::PythonModule::Destructor name='%1'").arg(name().c_str()) << endl;
#endif

    delete d;
}

Py::Dict PythonModule::getDict()
{
    return moduleDictionary();
}

Py::Object PythonModule::get(const Py::Tuple& args)
{
kdDebug() << "PythonModule::get() 1" << endl;

    if(args.size() < 1)
        throw Py::TypeError("Too few arguments.");
    if(args.size() > 1)
        throw Py::TypeError("Too many arguments.");
    if(! args[0].isString())
        throw Py::TypeError("String argument expected.");
    QString name = args[0].as_string().c_str();

kdDebug() << "PythonModule::get() 2" << endl;
//Kross::Api::Object::Ptr object = d->m_interpreter->m_manager->getChild( PythonExtension::toObject(args) );
Kross::Api::Object::Ptr object = static_cast<Kross::Api::Object*>(d->m_interpreter->m_manager)->getChild(name);
if(! object)
    throw Py::TypeError(QString("Unknown module '%1'.").arg(name).latin1());
kdDebug() << "PythonModule::get() 3" << endl;
return PythonExtension::toPyObject(object);

/*
Kross::Api::Object::Ptr module = d->m_interpreter->m_manager->getChild(name);
    //Kross::Api::Object::Ptr module = d->m_interpreter->m_manager->getModule(name);
    if(! module)
        throw Py::TypeError(QString("Unknown module '%1'.").arg(name).latin1());

    if(d->m_modules.contains(name))
        return Py::asObject(d->m_modules[name]);

    PythonExtension* pythonmodule = new PythonExtension(module);
    d->m_modules.replace(name, pythonmodule);
    return Py::asObject(pythonmodule);
*/
}

Py::Object PythonModule::import(const Py::Tuple& args)
{
    if(args.size() > 0) {
        QString modname = args[0].as_string().c_str();
        if(modname.startsWith("kross")) {
            kdDebug() << QString("PythonModule::import() module=%1").arg(modname) << endl;
            Kross::Api::Object::Ptr module = d->m_interpreter->m_manager->loadModule(modname);
            if(module)
                return PythonExtension::toPyObject(module);
        }
    }
    return Py::None();
}
