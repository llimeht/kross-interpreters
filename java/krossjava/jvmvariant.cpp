/***************************************************************************
 * jvmvariant.cpp
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

#include "jvmvariant.h"

using namespace Kross;

#if 0
VALUE JavaType<QVariant>::toVALUE(const QVariant& v)
{
    krossdebug( QString("JavaType<QVariant>::toVALUE variant.toString=%1 variant.typeid=%2 variant.typeName=%3").arg(v.toString()).arg(v.type()).arg(v.typeName()) );
    switch( v.type() ) {
        case QVariant::Int:
            return JavaType<int>::toVALUE(v.toInt());
        case QVariant::UInt:
            return JavaType<uint>::toVALUE(v.toUInt());
        case QVariant::Double:
            return JavaType<double>::toVALUE(v.toDouble());
        case QVariant::ByteArray:
            return JavaType<QByteArray>::toVALUE(v.toByteArray());
        case QVariant::String:
            return JavaType<QString>::toVALUE(v.toString());
        case QVariant::Bool:
            return JavaType<bool>::toVALUE(v.toBool());
        case QVariant::StringList:
            return JavaType<QStringList>::toVALUE(v.toStringList());
        case QVariant::Map:
            return JavaType<QVariantMap>::toVALUE(v.toMap());
        case QVariant::List:
            return JavaType<QVariantList>::toVALUE(v.toList());
        case QVariant::LongLong:
            return JavaType<qlonglong>::toVALUE(v.toLongLong());
        case QVariant::ULongLong:
            return JavaType<qlonglong>::toVALUE(v.toULongLong());

        case QVariant::Url:
            return JavaType<QUrl>::toVALUE(v.toUrl());
        case QVariant::Size:
            return JavaType<QSize>::toVALUE(v.toSize());
        case QVariant::SizeF:
            return JavaType<QSizeF>::toVALUE(v.toSizeF());
        case QVariant::Point:
            return JavaType<QPoint>::toVALUE(v.toPoint());
        case QVariant::PointF:
            return JavaType<QPointF>::toVALUE(v.toPointF());
        case QVariant::Rect:
            return JavaType<QRect>::toVALUE(v.toRect());
        case QVariant::RectF:
            return JavaType<QRectF>::toVALUE(v.toRectF());

        case QVariant::Invalid: {
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug( QString("JavaType<QVariant>::toVALUE variant=%1 is QVariant::Invalid. Returning Py:None.").arg(v.toString()) );
            #endif
            //return Py::None();
        } // fall through

        case QVariant::UserType: {
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug( QString("JavaType<QVariant>::toVALUE variant=%1 is QVariant::UserType. Trying to cast now.").arg(v.toString()) );
            #endif
        } // fall through

        default: {
            if( strcmp(v.typeName(),"float") == 0 ) {
                #ifdef KROSS_RUBY_VARIANT_DEBUG
                    krossdebug( QString("JavaType<QVariant>::toVALUE Casting '%1' to double").arg(v.typeName()) );
                #endif
                return JavaType<double>::toVALUE(v.toDouble());
            }

            if( qVariantCanConvert< QWidget* >(v) ) {
                #ifdef KROSS_RUBY_VARIANT_DEBUG
                    krossdebug( QString("JavaType<QVariant>::toVALUE Casting '%1' to QWidget").arg(v.typeName()) );
                #endif
                QWidget* widget = qvariant_cast< QWidget* >(v);
                if(! widget) {
                    #ifdef KROSS_RUBY_VARIANT_DEBUG
                        krosswarning( QString("JavaType<QVariant>::toVALUE To QWidget casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return 0;
                }
                return RubyExtension::toVALUE( new RubyExtension(widget) );
            }

            if( qVariantCanConvert< QObject* >(v) ) {
                #ifdef KROSS_RUBY_VARIANT_DEBUG
                    krossdebug( QString("JavaType<QVariant>::toVALUE Casting '%1' to QObject").arg(v.typeName()) );
                #endif
                QObject* obj = qvariant_cast< QObject* >(v);
                if(! obj) {
                    #ifdef KROSS_RUBY_VARIANT_DEBUG
                        krosswarning( QString("JavaType<QVariant>::toVALUE To QObject casted '%1' is NULL").arg(v.typeName()) );
                    #endif
                    return 0;
                }
                return RubyExtension::toVALUE( new RubyExtension(obj) );
            }

            //QObject* obj = (*reinterpret_cast< QObject*(*)>( variantargs[0]->toVoidStar() ));
            //PyObject* qobjectptr = PyLong_FromVoidPtr( (void*) variantargs[0]->toVoidStar() );

            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krosswarning( QString("JavaType<QVariant>::toVALUE Not possible to convert the QVariant '%1' with type '%2' (%3) to a VALUE.").arg(v.toString()).arg(v.typeName()).arg(v.type()) );
            #endif
            //throw Py::TypeError( QString("Variant of type %1 can not be casted to a python object.").arg(v.typeName()).toLatin1().constData() );
            return 0;
        }
    }
}

QVariant JavaType<QVariant>::toVariant(VALUE value)
{
    krossdebug(QString("JavaType<QVariant>::toVariant of type %1").arg(TYPE(value)));
    switch( TYPE( value ) )
    {
        case T_DATA: {
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a Kross::RubyExtension");
            #endif

            if(! RubyExtension::isRubyExtension(value)) {
                krosswarning("Cannot yet convert standard ruby type to Kross::RubyExtension object.");
                return 0;
            }

            RubyExtension* extension;
            Data_Get_Struct(value, RubyExtension, extension);
            Q_ASSERT(extension);
            QObject* object = extension->object();
            if(! object) {
                krossdebug("JavaType<QVariant>::toVariant QObject is NULL. Returning QVariant::Invalid.");
                return QVariant();
            }
            return qVariantFromValue( extension->object() );
        }

        case T_FLOAT:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_FLOAT");
            #endif
            return JavaType<double>::toVariant(value);
        case T_STRING:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_STRING");
            #endif
            return JavaType<QString>::toVariant(value);
        case T_ARRAY:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_ARRAY");
            #endif
            return JavaType<QVariantList>::toVariant(value);
        case T_FIXNUM:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_FIXNUM");
            #endif
            return JavaType<qlonglong>::toVariant(value);
        case T_HASH:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_HASH");
            #endif
            return JavaType<QVariantMap>::toVariant(value);
        case T_BIGNUM:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_BIGNUM");
            #endif
            return JavaType<qlonglong>::toVariant(value);

        case T_FALSE:
        case T_TRUE:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_TRUE/T_FALSE");
            #endif
            return JavaType<bool>::toVariant(value);

        case T_SYMBOL: // hmmm... where is this used?
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_SYMBOL");
            #endif
            return QString(rb_id2name(SYM2ID(value)));

        case T_NIL:
            #ifdef KROSS_RUBY_VARIANT_DEBUG
                krossdebug("  VALUE is a T_NIL. Returning QVariant::Invalid.");
            #endif
            return QVariant();

        case T_MATCH:
        case T_OBJECT:
        case T_FILE:
        case T_STRUCT:
        case T_REGEXP:
        case T_MODULE:
        case T_ICLASS:
        case T_CLASS:
        default:
            krosswarning(QString("This ruby type '%1' cannot be converted to a Kross::Object").arg(TYPE(value)));
            return QVariant();
    }
}

MetaType* RubyMetaTypeFactory::create(int typeId, int metaTypeId, VALUE value)
{
    krossdebug( QString("RubyMetaTypeFactory::create typeId=%1 typeName=%2").arg(QMetaType::typeName(typeId)).arg(typeId) );
    switch(typeId) {
        case QVariant::Int:
            return new RubyMetaTypeVariant<int>(value);
        case QVariant::UInt:
            return new RubyMetaTypeVariant<uint>(value);
        case QVariant::Double:
            return new RubyMetaTypeVariant<double>(value);
        case QVariant::Bool:
            return new RubyMetaTypeVariant<bool>(value);

        case QVariant::ByteArray:
            return new RubyMetaTypeVariant<QByteArray>(value);
        case QVariant::String:
            return new RubyMetaTypeVariant<QString>(value);

        case QVariant::StringList:
            return new RubyMetaTypeVariant<QStringList>(value);
        case QVariant::Map:
            return new RubyMetaTypeVariant<QVariantMap>(value);
        case QVariant::List:
            return new RubyMetaTypeVariant<QVariantList>(value);

        case QVariant::LongLong:
            return new RubyMetaTypeVariant<qlonglong>(value);
        case QVariant::ULongLong:
            return new RubyMetaTypeVariant<qulonglong>(value);

        case QVariant::Url:
            return new RubyMetaTypeVariant<QUrl>(value);
        case QVariant::Size:
            return new RubyMetaTypeVariant<QSize>(value);
        case QVariant::SizeF:
            return new RubyMetaTypeVariant<QSizeF>(value);
        case QVariant::Point:
            return new RubyMetaTypeVariant<QPoint>(value);
        case QVariant::PointF:
            return new RubyMetaTypeVariant<QPointF>(value);
        case QVariant::Rect:
            return new RubyMetaTypeVariant<QRect>(value);
        case QVariant::RectF:
            return new RubyMetaTypeVariant<QRectF>(value);

        case QVariant::Invalid: // fall through
        case QVariant::UserType: // fall through
        default: {
            if( RubyExtension::isRubyExtension(value) ) {
                #ifdef KROSS_RUBY_VARIANT_DEBUG
                    krossdebug( QString("RubyMetaTypeFactory::create VALUE with typeId '%1' is a RubyExtension object").arg(typeId) );
                #endif

                RubyExtension* extension;
                Data_Get_Struct(value, RubyExtension, extension);
                Q_ASSERT(extension);

                QObject* object = extension->object();
                if(! object) {
                    krosswarning("RubyMetaTypeFactory::create QObject is NULL.");
                    return 0;
                }

                return new MetaTypeVoidStar( typeId, object, false );
            }

            if( TYPE(value) == T_NIL ) {
                #ifdef KROSS_PYTHON_VARIANT_DEBUG
                    krossdebug( QString("RubyMetaTypeFactory::create VALUE is T_NIL. Create empty type '%1'").arg(metaTypeId) );
                #endif
                void* ptr = QMetaType::construct(metaTypeId, 0);
                return new MetaTypeVoidStar( metaTypeId, ptr, false );
            }

            //QVariant v = JavaType<QVariant>::toVariant(object);
            //krossdebug( QString("RubyVariant::create Converted VALUE '%1' with type '%2 %3' to QVariant with type '%4 %5'").arg(object.as_string().c_str()).arg(typeName).arg(typeId).arg(v.toString()).arg(v.typeName()) );
            //if(typeId == QVariant::Invalid) return new RubyVariantImpl<void>();
            //return new RubyVariantImpl<QVariant>(v);

            krosswarning( QString("RubyMetaTypeFactory::create Not possible to convert the VALUE to QVariant with '%1' and metaid '%2'").arg(QVariant::typeToName((QVariant::Type)typeId)).arg(typeId) );
            return 0;
        } break;
    }
}
#endif