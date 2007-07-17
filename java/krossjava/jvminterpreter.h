/***************************************************************************
 * jvminterpreter.h
 * This file is part of the KDE project
 *
 * copyright (C)2007 by Vincent Verhoeven <verhoevenv@gmail.com>
 * copyright (C)2007 by Sebastian Sauer <mail@dipe.org>
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

#ifndef KROSS_JVMINTERPRETER_H
#define KROSS_JVMINTERPRETER_H

#include "jvmconfig.h"

#include <kross/core/interpreter.h>

#include <QString>

namespace Kross {

    // Forward declarations.
    class Script;
    class Action;
    class JVMScript;
    class JVMModule;
    class JVMExtension;
    class JVMInterpreterPrivate;

    /**
    * The JVMInterpreter class implements a \a Kross::Interpreter for
    * the Java backend. At each time there exist exactly one instance
    * of the interpreter.
    */
    class JVMInterpreter : public Interpreter
    {
        public:

            /**
            * Constructor.
            * \param info the \a InterpreterInfo instance the Kross core
            * uses to provide details about the Java-interpreter without
            * actualy loading the krossjava library.
            */
            explicit JVMInterpreter(InterpreterInfo* info);

            /**
            * Destructor.
            */
            virtual ~JVMInterpreter();

            /**
            * Factory method that returns a \a JVMScript instance. There
            * can be 0..n instances of \a JVMScript around where each of
            * them represents an independend java project.
            *
            * \param action The \a Kross::Action that contains details
            * about the actual Java code we got e.g. from an application
            * that uses Kross.
            * \return The new \a JVMScript instance that implements a
            * abstract script container for the java language.
            */
            virtual Script* createScript(Action* action);

            //This should probably become a more local class
            static JNIEnv* getEnv();
            static bool addClass(const QString& name, const QByteArray& array);
            static void addToCP(const QUrl& url);
            static jobject newObject(const QString& name);
            static jobject addExtension(const QString& name, const JVMExtension* obj, const QByteArray& clazz, const QObject* wrapped);
            static const JVMExtension* extension(const QObject* obj);
            //TODO: would this be the right place?
            static bool handleException();
#ifdef KROSS_JVM_INTERPRETER_DEBUG
            static void showDebugInfo(jobject obj);
#endif

        private:
            /// \internal d-pointer class.
            class Private;

            //TODO: is it okay for a d-pointer to be static?
            //sebsauer: normaly it isn't since if there are multiple instances of the JVMInterpreter
            //          around they would need to share it. While in our case the JVMInterpreter class
            //          is a singleton and therefore static should be ok, it may better to use
            //          K_GLOBAL_STATIC ( http://www.englishbreakfastnetwork.org/apidocs/apidox-kde-4.0/kdelibs-apidocs/kdecore/html/group__KDEMacros.html#g75ca0c60b03dc5e4f9427263bf4043c7 )
            //          but JNIEnv* shouldn't be static anyway.

            /// \internal d-pointer instance.
            static Private * const d;
    };

}

#endif
