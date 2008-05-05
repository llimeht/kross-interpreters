/***************************************************************************
 * pythonvariant.cpp
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

#include "pythonvariant.h"
#include "pythonextension.h"

//#include <kross/core/variant.h>

#include <QWidget>

using namespace Kross;

Py::Object PythonType<QVariant>::toPyObject(const QVariant& v)
{
    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonType<QVariant>::toPyObject variant.toString=%1 variant.typeid=%2 variant.typeName=%3").arg(v.toString()).arg(v.type()).arg(v.typeName()) );
    #endif

    switch( v.type() ) {
        case QVariant::Int:
            return PythonType<int>::toPyObject(v.toInt());
        case QVariant::UInt:
            return PythonType<uint>::toPyObject(v.toUInt());
        case QVariant::Double:
            return PythonType<double>::toPyObject(v.toDouble());
        case QVariant::ByteArray:
            return PythonType<QByteArray>::toPyObject(v.toByteArray());
        case QVariant::String:
            return PythonType<QString>::toPyObject(v.toString());
        case QVariant::Bool:
            return PythonType<bool>::toPyObject(v.toBool());
        case QVariant::StringList:
            return PythonType<QStringList>::toPyObject(v.toStringList());
        case QVariant::Map:
            return PythonType<QVariantMap>::toPyObject(v.toMap());
        case QVariant::List:
            return PythonType<QVariantList>::toPyObject(v.toList());
        case QVariant::LongLong:
            return PythonType<qlonglong>::toPyObject(v.toLongLong());
        case QVariant::ULongLong:
            return PythonType<qlonglong>::toPyObject(v.toULongLong());

        case QVariant::Size:
            return PythonType<QSize>::toPyObject(v.toSize());
        case QVariant::SizeF:
            return PythonType<QSizeF>::toPyObject(v.toSizeF());
        case QVariant::Point:
            return PythonType<QPoint>::toPyObject(v.toPoint());
        case QVariant::PointF:
            return PythonType<QPointF>::toPyObject(v.toPointF());
        case QVariant::Rect:
            return PythonType<QRect>::toPyObject(v.toRect());
        case QVariant::RectF:
            return PythonType<QRectF>::toPyObject(v.toRectF());

        case QVariant::Color:
            return PythonType<QColor>::toPyObject(v.value<QColor>());
        case QVariant::Url:
            return PythonType<QUrl>::toPyObject(v.toUrl());

        case QVariant::Date:
            return PythonType<QDate>::toPyObject( v.value<QDate>() );
        case QVariant::Time:
            return PythonType<QTime>::toPyObject( v.value<QTime>() );
        case QVariant::DateTime:
            return PythonType<QDateTime>::toPyObject( v.value<QDateTime>() );

        case QVariant::Invalid: {
            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krossdebug( QString("PythonType<QVariant>::toPyObject variant=%1 is QVariant::Invalid. Returning Py:None.").arg(v.toString()) );
            #endif
            //return Py::Object();
            return Py::None(); //FIXME should we fall through here?
        } break;

        case QVariant::UserType: {
            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krossdebug( QString("PythonType<QVariant>::toPyObject variant=%1 is QVariant::UserType. Trying to cast now.").arg(v.toString()) );
            #endif
        } // fall through

        default: {
            if( strcmp(v.typeName(),"float") == 0 ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to double").arg(v.typeName()) );
                #endif
                return PythonType<double>::toPyObject(v.toDouble());
            }

            if( qVariantCanConvert< Kross::Object::Ptr >(v) ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to Kross::Object::Ptr").arg(v.typeName()) );
                #endif
                Kross::Object::Ptr obj = v.value< Kross::Object::Ptr >();
                Kross::PythonObject* pyobj = dynamic_cast< Kross::PythonObject* >(obj.data());
                if(! obj) {
                    #ifdef KROSS_PYTHON_VARIANT_DEBUG
                        krossdebug( QString("PythonType<QVariant>::toPyObject To Kross::PythonObject* casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return Py::None();
                }
                return pyobj->pyObject();
            }

            if( qVariantCanConvert< QWidget* >(v) ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to QWidget").arg(v.typeName()) );
                #endif
                QWidget* widget = qvariant_cast< QWidget* >(v);
                if(! widget) {
                    #ifdef KROSS_PYTHON_VARIANT_DEBUG
                        krossdebug( QString("PythonType<QVariant>::toPyObject To QWidget casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return Py::None();
                }
                return Py::asObject(new PythonExtension(widget));
            }

            if( qVariantCanConvert< QObject* >(v) ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to QObject").arg(v.typeName()) );
                #endif
                QObject* obj = qvariant_cast< QObject* >(v);
                if(! obj) {
                    #ifdef KROSS_PYTHON_VARIANT_DEBUG
                        krossdebug( QString("PythonType<QVariant>::toPyObject To QObject casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return Py::None();
                }
                return Py::asObject(new PythonExtension(obj));
            }

            //QObject* obj = (*reinterpret_cast< QObject*(*)>( variantargs[0]->toVoidStar() ));
            //PyObject* qobjectptr = PyLong_FromVoidPtr( (void*) variantargs[0]->toVoidStar() );

            //if(v.type() == QVariant::Invalid) return Py::None();

            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krosswarning( QString("PythonType<QVariant>::toPyObject Not possible to convert the QVariant '%1' with type '%2' (%3) to a Py::Object.").arg(v.toString()).arg(v.typeName()).arg(v.type()) );
            #endif
            throw Py::TypeError( QString("Variant of type %1 can not be casted to a python object.").arg(v.typeName()).toLatin1().constData() );
        }
    }
}

QVariant PythonType<QVariant>::toVariant(const Py::Object& obj)
{
    if(obj == Py::None()) {
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
            krossdebug( QString("PythonType<QVariant>::toVariant Py::None") );
        #endif
        return QVariant();
    }

    PyTypeObject *type = (PyTypeObject*) obj.type().ptr();
    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonType<QVariant>::toVariant type=%1").arg(type->tp_name) );
    #endif

    if(type == &PyInt_Type)
        return PythonType<int>::toVariant(obj);
    if(type == &PyLong_Type)
        return PythonType<qlonglong>::toVariant(obj);
    if(type == &PyFloat_Type)
        return PythonType<double>::toVariant(obj);
    if(type == &PyBool_Type)
        return PythonType<bool>::toVariant(obj);
    if(obj.isString())
        return PythonType<QString>::toVariant(obj);
    if(type == &PyTuple_Type)
        return PythonType<QVariantList,Py::Tuple>::toVariant(Py::Tuple(obj));
    if(type == &PyList_Type)
        return PythonType<QVariantList,Py::List>::toVariant(Py::List(obj));
    if(type == &PyDict_Type)
        return PythonType<QVariantMap,Py::Dict>::toVariant(Py::Dict(obj.ptr()));

    if(obj.isInstance()) {
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonType<QVariant>::toVariant IsInstance=TRUE") );
        #endif
        //return new PythonType(object);
        QVariant result;
        Kross::Object::Ptr p;
        p.attach(new Kross::PythonObject(obj));
        result.setValue(p);
        return result;
    }

    if(PythonExtension::check(obj.ptr())) {
        Py::ExtensionObject<PythonExtension> extobj(obj);
        PythonExtension* extension = extobj.extensionObject();
        if(! extension) {
            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krosswarning( QString("PythonType<QVariant>::toVariant Failed to determinate PythonExtension for object=%1").arg(obj.as_string().c_str()) );
            #endif
            throw Py::RuntimeError( QString("Failed to determinate PythonExtension object.").toLatin1().constData() );
        }

        const QVariant variant = qVariantFromValue( extension->object() );
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
            if(extension->object())
                krossdebug( QString("PythonType<QVariant>::toVariant KrossObject.objectName=%1 KrossObject.className=%2 QVariant.toString=%3 QVariant.typeName=%4").arg(extension->object()->objectName()).arg(extension->object()->metaObject()->className()).arg(variant.toString()).arg(variant.typeName()) );
            else
                krossdebug( QString("PythonType<QVariant>::toVariant The PythonExtension object does not have a valid QObject") );
        #endif
        return variant;
    }

    throw Py::RuntimeError( QString("Invalid object of type '%1'.").arg(type->tp_name).toLatin1().constData() );
    return QVariant();
}

#if 0
Py::Object PythonType<QColor>::toPyObject(const QColor& color)
{
    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonType<QColor>::toPyObject color.name=%1").arg(color.name()) );
    #endif
krossdebug( QString("....1") );
    Color* c = new Color(0 /*no parent*/, color);
krossdebug( QString("....2") );
    Py::Object o = Py::asObject( new PythonExtension(c, false /*owner*/) );
krossdebug( QString("....3") );
    return o;
}

QColor PythonType<QColor>::toVariant(const Py::Object& obj)
{
    if( PythonExtension::check(obj.ptr()) ) {
        Py::ExtensionObject<PythonExtension> extobj(obj);
        Q_ASSERT( extobj.extensionObject() );
        Color* color = dynamic_cast< Color* >( extobj.extensionObject()->object() );
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
            krossdebug( QString("PythonType<QColor>::toVariant Color.name=%1").arg(color ? color->name() : "NULL") );
        #endif
        if( color ) return color->color();
    }
    PyTypeObject *type = (PyTypeObject*) obj.type().ptr();
    if( PyType_IsSubtype(type,&PyString_Type) ) {
        QString cname = PythonType<QString>::toVariant(obj);
        QColor c(cname);
        #ifdef KROSS_PYTHON_VARIANT_DEBUG
            krossdebug( QString("PythonType<QColor>::toVariant string=%1 color.name=%2").arg(cname).arg(c.name()) );
        #endif
        return c;
    }
    if( type == &PyTuple_Type || type == &PyList_Type ) {
        Py::Tuple t(obj);
        if( t.size() >= 3 ) {
            bool f = ( ((PyTypeObject*)(t[0].type().ptr())) == &PyFloat_Type );
            int r = f ? int(PythonType<double>::toVariant(t[0]) * 255.0) : PythonType<int>::toVariant(t[0]);
            int g = f ? int(PythonType<double>::toVariant(t[1]) * 255.0) : PythonType<int>::toVariant(t[1]);
            int b = f ? int(PythonType<double>::toVariant(t[2]) * 255.0) : PythonType<int>::toVariant(t[2]);
            int a = (t.size() >= 4) ? ( f ? int(PythonType<double>::toVariant(t[3]) * 255.0) : PythonType<int>::toVariant(t[3]) ) : 255;
            QColor c(r,g,b,a);
            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krossdebug( QString("PythonType<QColor>::toVariant Tuple QColor.name=%1").arg(c.name()) );
            #endif
            return c;
        }
    }
    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonType<QColor>::toVariant empty QColor()") );
    #endif
    return QColor();
}
#endif

MetaType* PythonMetaTypeFactory::create(const char* typeName, const Py::Object& object, bool owner)
{
    int typeId = QVariant::nameToType(typeName);

    #ifdef KROSS_PYTHON_VARIANT_DEBUG
        krossdebug( QString("PythonMetaTypeFactory::create typeName=%1 metatype.id=%2 variant.id=%3 object=%4").arg(typeName).arg(QMetaType::type(typeName)).arg(typeId).arg(object.as_string().c_str()) );
    #endif

    switch(typeId) {
        case QVariant::Int:
            return new PythonMetaTypeVariant<int>(object);
        case QVariant::UInt:
            return new PythonMetaTypeVariant<uint>(object);
        case QVariant::Double:
            return new PythonMetaTypeVariant<double>(object);
        case QVariant::Bool:
            return new PythonMetaTypeVariant<bool>(object);

        case QVariant::ByteArray:
            return new PythonMetaTypeVariant<QByteArray>(object);
        case QVariant::String:
            return new PythonMetaTypeVariant<QString>(object);

        case QVariant::StringList:
            return new PythonMetaTypeVariant<QStringList>(object);
        case QVariant::Map:
            return new PythonMetaTypeVariant<QVariantMap>(object);
        case QVariant::List:
            return new PythonMetaTypeVariant<QVariantList>(object);

        case QVariant::LongLong:
            return new PythonMetaTypeVariant<qlonglong>(object);
        case QVariant::ULongLong:
            return new PythonMetaTypeVariant<qulonglong>(object);

        case QVariant::Size:
            return new PythonMetaTypeVariant<QSize>(object);
        case QVariant::SizeF:
            return new PythonMetaTypeVariant<QSizeF>(object);
        case QVariant::Point:
            return new PythonMetaTypeVariant<QPoint>(object);
        case QVariant::PointF:
            return new PythonMetaTypeVariant<QPointF>(object);
        case QVariant::Rect:
            return new PythonMetaTypeVariant<QRect>(object);
        case QVariant::RectF:
            return new PythonMetaTypeVariant<QRectF>(object);

        case QVariant::Color:
            return new PythonMetaTypeVariant<QColor>(object);
        case QVariant::Url:
            return new PythonMetaTypeVariant<QUrl>(object);

        case QVariant::Date:
            return new PythonMetaTypeVariant<QDate>(object);
        case QVariant::Time:
            return new PythonMetaTypeVariant<QTime>(object);
        case QVariant::DateTime:
            return new PythonMetaTypeVariant<QDateTime>(object);

        case QVariant::Invalid: // fall through
        case QVariant::UserType: // fall through
        default: {
            if(Py::PythonExtension<PythonExtension>::check( object )) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonMetaTypeFactory::create Py::Object '%1' with typename '%2' is a PythonExtension object").arg(object.as_string().c_str()).arg(typeName) );
                #endif

                Py::ExtensionObject<PythonExtension> extobj(object);
                PythonExtension* extension = extobj.extensionObject();
                if( ! extension->object() )
                    throw Py::RuntimeError( QString("Underlying QObject instance of the PythonExtension was removed.").toLatin1().constData() );
                return new MetaTypeVoidStar( typeId, extension->object(), owner );
            }

            /*
            //if( metaid == QMetaType::QObjectStar )
            if( metaid == QMetaType::QWidgetStar ) {
                if( qVariantCanConvert< QWidget* >(v) ) {
                    QWidget* widget = qvariant_cast< QWidget* >(v);
                    return new MetaTypeVoidStar( metaid, widget, false );
                }
            }
            */

            int metaid = QMetaType::type(typeName);
            if( metaid > 0 ) {
                switch(metaid) {
                    case QMetaType::QObjectStar: // fall through
                    case QMetaType::QWidgetStar: {
                        Py::Object pyobjtype( PyObject_Type(object.ptr()), true /* owner */ );
                        const QString pyobjtypename( pyobjtype.repr().as_string().c_str() );
                        #ifdef KROSS_PYTHON_VARIANT_DEBUG
                            krossdebug( QString("PythonMetaTypeFactory::create Py::Object is %1").arg(pyobjtypename) );
                        #endif
                        if( pyobjtypename.startsWith("<class 'PyQt4.") ) {
                            try {
                                // We got a PyQt4 class and are using now the sip module
                                // to unwrapinstance those PyQt4 QObject/QWidget.
                                Py::Module mainmod( PyImport_AddModule( (char*)"sip" ) );
                                Py::Callable func = mainmod.getDict().getItem("unwrapinstance");
                                Py::Tuple arguments(1);
                                arguments[0] = object; //pyqtobject pointer
                                Py::Object result = func.apply(arguments); // call the sip.unwrapinstance function
                                void* ptr = PyLong_AsVoidPtr( result.ptr() ); // the result is a void* pointer to our QObject/QWidget instance
                                QObject* obj = 0;
                                switch(metaid) {
                                    case QMetaType::QObjectStar: {
                                        obj = static_cast<QObject*>(ptr);
                                        if( obj && ! obj->parent() ) {
                                            // This is a dirty hack to provide a parent to the passed
                                            // QObject/QWidget instance to be sure it doesn't went away.
                                            QObject* parent = new QObject();
                                            QObject::connect(obj, SIGNAL(destroyed(QObject*)), parent, SLOT(deleteLater()));
                                            obj->setParent(parent);
                                        }
                                    } break;
                                    case QMetaType::QWidgetStar: {
                                        obj = static_cast<QWidget*>(ptr);
                                        if( obj && ! obj->parent() ) {
                                            QWidget* parent = new QWidget();
                                            QObject::connect(obj, SIGNAL(destroyed(QObject*)), parent, SLOT(deleteLater()));
                                            static_cast<QWidget*>(obj)->setParent(parent);
                                       }
                                    } break;
                                    default:
                                        break;
                                }
                                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                                    krossdebug(QString("PythonMetaTypeFactory::create class=%1 ptr=%2").arg(obj ? obj->metaObject()->className() : "NULL").arg(long(ptr)));
                                #endif
                                return new MetaTypeVoidStar( metaid, obj, false /*owner*/ );
                            }
                            catch(Py::Exception& e) {
                                krosswarning(QString("PythonMetaTypeFactory::create EXCEPTION %1").arg(Py::value(e).as_string().c_str()));
                            }
                        }
                        #ifdef KROSS_PYTHON_VARIANT_DEBUG
                            krossdebug( QString("PythonMetaTypeFactory::create Py::Object isNone. Create empty type '%1'").arg(metaid) );
                        #endif
                        void* ptr = 0; //QMetaType::construct(metaid, 0);
                        return new MetaTypeVoidStar( metaid, ptr, false /*owner*/ );
                    } break;
                    default:
                        break;
                }

                // this is a dirty hack to downcast KUrl's to QUrl's
                // without the need to link against kdelibs.
                if( strcmp(typeName,"KUrl") == 0 ) {
                    return new PythonMetaTypeVariant<QUrl>(object);
                }
            }

            QVariant v = PythonType<QVariant>::toVariant(object);

            if( qVariantCanConvert< Kross::Object::Ptr >(v) ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("PythonType<QVariant>::toPyObject Casting '%1' to Kross::Object::Ptr").arg(v.typeName()) );
                #endif
                if( Kross::Object::Ptr ptr = v.value< Kross::Object::Ptr >() )
                    return new MetaTypeVariant<Kross::Object::Ptr>(ptr);
            }

            #ifdef KROSS_PYTHON_VARIANT_DEBUG
                krosswarning( QString("PythonMetaTypeFactory::create Convert Py::Object '%1' to QVariant v.toString='%2' v.typeName='%3' typeName='%4' typeId='%5'").arg(object.as_string().c_str()).arg(v.toString()).arg(v.typeName()).arg(typeName).arg(typeId) );
            #endif
            return new Kross::MetaTypeVariant< QVariant >( v );
        } break;
    }
}

