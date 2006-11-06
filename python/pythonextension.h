/***************************************************************************
 * pythonextension.h
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_PYTHONEXTENSION_H
#define KROSS_PYTHONEXTENSION_H

#include "pythonconfig.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QVariant>
#include <QPointer>

namespace Kross {

    // Forward declaration.
    class PythonExtension;
    class PythonScript;

    /**
     * The PythonExtension class implements a Py::Object to wrap a
     * QObject instance into the world of Python.
     */
    class PythonExtension : public Py::PythonExtension<PythonExtension>
    {
            friend class PythonScript;
            //friend class PythonModule;
        public:

            /**
             * Constructor.
             *
             * \param object The QObject this extension instance wraps.
             */
            explicit PythonExtension(QObject* object);

            /**
             * Destructor.
             */
            virtual ~PythonExtension();

            /**
             * \return the QObject instance this extension instance wraps.
             */
            QObject* object() const;

            /**
             * Handle getting of attributes. An attribute could be a property
             * as well as a pointer to a callable memberfunction.
             *
             * \param name The name of the attribute that should be handled.
             * \return An \a Py::Object that could be a value or a callable
             * object. Python will decide what to do with the returnvalue.
             */
            virtual Py::Object getattr(const char* name);
            //virtual Py::Object getattr_methods(const char* name);

            /**
             * Handle setting of attributes.
             *
             * \param name The name of the attribute.
             * \param value The value to set the attribute.
             * \return -1 on failure and 0 on success.
             */
            virtual int setattr(const char* name, const Py::Object& value);

            virtual int sequence_length();
            virtual Py::Object sequence_concat(const Py::Object&);
            virtual Py::Object sequence_repeat(int);
            virtual Py::Object sequence_item(int);
            virtual Py::Object sequence_slice(int, int);
            virtual int sequence_ass_item(int, const Py::Object&);
            virtual int sequence_ass_slice(int, int, const Py::Object&);

            virtual int mapping_length();
            virtual Py::Object mapping_subscript(const Py::Object&);
            virtual int mapping_ass_subscript(const Py::Object&, const Py::Object&);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;

            /// Return the name of the QObject class.
            Py::Object getClassName(const Py::Tuple&);
            /// Return list of signal names the QObject provides.
            Py::Object getSignalNames(const Py::Tuple&);
            /// Return list of slot names the QObject provides.
            Py::Object getSlotNames(const Py::Tuple&);
            /// Return list of property names the QObject provides.
            Py::Object getPropertyNames(const Py::Tuple&);
            /// Return a property value.
            Py::Object getProperty(const Py::Tuple&);
            /// Set a property value.
            Py::Object setProperty(const Py::Tuple&);

            //Py::Object toPointer(const Py::Tuple&);
            //Py::Object fromPointer(const Py::Tuple&);

            /// Connect signal, slots or python functions together.
            Py::Object doConnect(const Py::Tuple&);
            /// Disconnect signal, slots or python functions that are connected together.
            Py::Object doDisconnect(const Py::Tuple&);

            /**
             * The static proxy-handler which will be used to dispatch
             * a call to our \a PythonExtension instance and redirect
             * the call to the matching method.
             *
             * \param _self_and_name_tuple A tuple containing as first
             *        argument a reference to our \a PythonExtension
             *        instance.
             * \param _args The optional passed arguments for the method
             *        which should be called.
             * \return The returnvalue of the methodcall.
             */
            static PyObject* proxyhandler(PyObject* _self_and_name_tuple, PyObject* _args);
    };

}

#endif
