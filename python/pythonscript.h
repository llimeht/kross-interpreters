/***************************************************************************
 * pythonscript.h
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

#ifndef KROSS_PYTHON_PYTHONSCRIPT_H
#define KROSS_PYTHON_PYTHONSCRIPT_H

#include <Python.h>
#include "CXX/Objects.hxx"
//#include "CXX/Extensions.hxx"

//#include <qstring.h>
//#include <qvariant.h>
//#include <qobject.h>
//#include <kdebug.h>
#include "../api/script.h"

namespace Kross { namespace Python {

    // Forward declaration.
    class PythonModuleManager;

    class PythonScript : public Kross::Api::Script
    {
        public:
            explicit PythonScript(Kross::Api::Interpreter* interpreter, Kross::Api::ScriptContainer* scriptcontainer);
            virtual ~PythonScript();

            virtual const QStringList& getFunctionNames();
            virtual Kross::Api::Object* execute();
            virtual Kross::Api::Object* callFunction(const QString& name, Kross::Api::List* args);

        private:
            Py::Module* m_module;

            void initialize();
            void finalize();

    };

}}

#endif
