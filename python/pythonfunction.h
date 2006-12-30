/***************************************************************************
 * pythonfunction.h
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
 *
 * Parts of the code are from kjsembed4 SlotProxy
 * Copyright (C) 2005, 2006 KJSEmbed Authors.
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

#ifndef KROSS_PYTHONFUNCTION_H
#define KROSS_PYTHONFUNCTION_H

#include "pythonconfig.h"

#include <QObject>

class QMetaObject;
class QByteArray;

namespace Kross {

    /**
     * The PythonFunction class implements a QObject to provide
     * an adaptor between Qt signals+slots and python functions.
     */
    class PythonFunction : public QObject
    {
        public:

            /**
            * Constructor.
            *
            * \param sender The QObject instance that sends the signal.
            * \param signal The signature of the signal the QObject emits.
            * \param callable The callable python function that should
            * be executed if the QObject emits the signal.
            */
            PythonFunction(QObject* sender, const QByteArray& signal, const Py::Callable& callable);

            /**
            * Destructor.
            */
            virtual ~PythonFunction();

            /**
            * The static \a QMetaObject instance that provides the
            * QMeta-information for this QObject class.
            */
            QMetaObject staticMetaObject;

            /**
            * \return the dynamic build \a QMetaObject instance
            * for this QObject instance.
            */
            const QMetaObject *metaObject() const;

            /**
            * Try to cast this QObject instance into the class with
            * name \p _clname and return the casted pointer or NULL
            * if casting failed.
            */
            void *qt_metacast(const char *_clname);

            /**
            * This method got called if a method this QObject instance
            * defines should be invoked.
            */
            int qt_metacall(QMetaObject::Call _c, int _id, void **_a);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif