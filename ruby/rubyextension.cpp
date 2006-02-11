/***************************************************************************
 * rubyinterpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
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
#include "rubyextension.h"

#include <st.h>

#include <qmap.h>
#include <qstring.h>

#include "api/list.h"

#include "rubyconfig.h"

namespace Kross {

namespace Ruby {

    
class RubyExtensionPrivate {
    friend class RubyExtension;
    /// The \a Kross::Api::Object this RubyExtension wraps.
    Kross::Api::Object::Ptr m_object;
    /// 
    static VALUE s_krossObject;
};

VALUE RubyExtensionPrivate::s_krossObject = 0;
    
VALUE RubyExtension::method_missing(int argc, VALUE *argv, VALUE self)
{
#ifdef KROSS_RUBY_EXTENSION_DEBUG
    kdDebug() << "method_missing(argc, argv, self)" << endl;
#endif
    if(argc < 1)
    {
        return 0;
    }
#ifdef KROSS_RUBY_EXTENSION_DEBUG
    kdDebug() << "Converting self to Kross::Api::Object" << endl;
#endif
    
    Kross::Api::Object::Ptr object = toObject( self );
    return RubyExtension::call_method(object, argc, argv);
}

VALUE RubyExtension::call_method( Kross::Api::Object::Ptr object, int argc, VALUE *argv)
{
    QString funcname = rb_id2name(SYM2ID(argv[0]));
    QValueList<Api::Object::Ptr> argsList;
#ifdef KROSS_RUBY_EXTENSION_DEBUG
    kdDebug() << "Building arguments list for function : " << funcname << " there are " << (argc-1) << " arguments." << endl;
#endif
    for(int i = 1; i < argc; i++)
    {
        Kross::Api::Object::Ptr obj = toObject(argv[i]);
        if(obj) argsList.append(obj);
    }
    Kross::Api::Object::Ptr result;
    if(object->hasChild(funcname)) {
#ifdef KROSS_RUBY_EXTENSION_DEBUG
        kdDebug() << QString("Kross::Ruby::RubyExtension::method_missing name='%1' is a child object of '%2'.").arg(funcname).arg(object->getName()) << endl;
#endif
        result = object->getChild(funcname)->call(QString::null, new Api::List(argsList));
    }
    else {
#ifdef KROSS_RUBY_EXTENSION_DEBUG
        kdDebug() << QString("Kross::Ruby::RubyExtension::method_missing try to call function with name '%1' in object '%2'.").arg(funcname).arg(object->getName()) << endl;
#endif
        result = object->call(funcname, new Api::List(argsList));
    }
    
    return toVALUE(result);
}
    
RubyExtension::RubyExtension(Kross::Api::Object::Ptr object) : d(new RubyExtensionPrivate())
{
    d->m_object = object;
}


RubyExtension::~RubyExtension()
{
}

typedef QMap<QString, Kross::Api::Object::Ptr> mStrObj;

int RubyExtension::convertHash_i(VALUE key, VALUE value, VALUE  vmap)
{
    QMap<QString, Kross::Api::Object::Ptr>* map; 
    Data_Get_Struct(vmap, mStrObj, map);
    if (key != Qundef)
    {
        Kross::Api::Object::Ptr o = RubyExtension::toObject( value );
        if(o) map->replace(STR2CSTR(key), o);
    }
    return ST_CONTINUE;
}

Kross::Api::Object::Ptr RubyExtension::toObject(VALUE value)
{
#ifdef KROSS_RUBY_EXTENSION_DEBUG
    kdDebug() << "RubyExtension::toObject of type " << TYPE(value) << endl;
#endif
    switch( TYPE( value ) )
    {
        case T_DATA:
        {
#ifdef KROSS_RUBY_EXTENSION_DEBUG
            kdDebug() << "Object is a Kross Object" << endl;
#endif
            VALUE result = rb_funcall(value, rb_intern("kind_of?"), 1, RubyExtensionPrivate::s_krossObject );
            if(TYPE(result) == T_TRUE)
            {
                RubyExtension* objectExtension;
                Data_Get_Struct(value, RubyExtension, objectExtension);
                Kross::Api::Object::Ptr object = objectExtension->d->m_object;
                return object;
            } else {
                kdWarning() << "Cannot yet convert standard ruby type to kross object" << endl;
                return 0;
            }
        }
        case T_FLOAT:
            return new Kross::Api::Variant(NUM2DBL(value));
        case T_STRING:
            return new Kross::Api::Variant(QString(STR2CSTR(value)));
        case T_ARRAY:
        {
            QValueList<Kross::Api::Object::Ptr> l;
            for(int i = 0; i < RARRAY(value)->len; i++)
            {
                Kross::Api::Object::Ptr o = toObject( rb_ary_entry( value , i ) );
                if(o) l.append(o);
            }
            return new Kross::Api::List(l);
        }
        case T_FIXNUM:
            return new Kross::Api::Variant((Q_LLONG)FIX2INT(value));
        case T_HASH:
        {
            QMap<QString, Kross::Api::Object::Ptr> map;
            VALUE vmap = Data_Wrap_Struct(rb_cObject, 0,0, &map);
            rb_hash_foreach(value, (int (*)(...))convertHash_i, vmap);
            return new Kross::Api::Dict(map);
        }
        case T_BIGNUM:
        {
            return new Kross::Api::Variant((Q_LLONG)NUM2LONG(value));
        }
        case T_TRUE:
        {
            return new Kross::Api::Variant(true);
        }
        case T_FALSE:
        {
            return new Kross::Api::Variant(false);
        }
        case T_SYMBOL:
        {
            return new Kross::Api::Variant(QString(rb_id2name(SYM2ID(value))));
        }
        case T_MATCH:
        case T_OBJECT:
        case T_FILE:
        case T_STRUCT:
        case T_REGEXP:
        case T_MODULE:
        case T_ICLASS:
        case T_CLASS:
            kdWarning() << QString("This ruby type '%1' cannot be converted to a Kross::Api::Object").arg(TYPE(value)) << endl;
        default:
        case T_NIL:
            return 0;
    }
}

VALUE RubyExtension::toVALUE(const QString& s)
{
    return s.isNull() ? rb_str_new2("") : rb_str_new2(s.latin1());
}

VALUE RubyExtension::toVALUE(QStringList list)
{
    VALUE l = rb_ary_new();
    for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        rb_ary_push(l, toVALUE(*it));
    return l;
}


VALUE RubyExtension::toVALUE(QMap<QString, QVariant> map)
{
    VALUE h = rb_hash_new();
    for(QMap<QString, QVariant>::Iterator it = map.begin(); it != map.end(); ++it)
        rb_hash_aset(h, toVALUE(it.key()), toVALUE(it.data()) );
    return h;

}

VALUE RubyExtension::toVALUE(QValueList<QVariant> list)
{
    VALUE l = rb_ary_new();
    for(QValueList<QVariant>::Iterator it = list.begin(); it != list.end(); ++it)
        rb_ary_push(l, toVALUE(*it));
    return l;
}


VALUE RubyExtension::toVALUE(const QVariant& variant)
{
    
    switch(variant.type()) {
        case QVariant::Invalid:
            return Qnil;
        case QVariant::Bool:
            return (variant.toBool()) ? Qtrue : Qfalse;
        case QVariant::Int:
            return INT2FIX(variant.toInt());
        case QVariant::UInt:
            return UINT2NUM(variant.toUInt());
        case QVariant::Double:
            return rb_float_new(variant.toDouble());
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        case QVariant::ByteArray:
        case QVariant::BitArray:
        case QVariant::CString:
        case QVariant::String:
            return toVALUE(variant.toString());
        case QVariant::StringList:
            return toVALUE(variant.toStringList());
        case QVariant::Map:
            return toVALUE(variant.toMap());
        case QVariant::List:
            return toVALUE(variant.toList());

        // To handle following both cases is a bit difficult
        // cause Python doesn't spend an easy possibility
        // for such large numbers (TODO maybe BigInt?). So,
        // we risk overflows here, but well...
        case QVariant::LongLong: {
            return INT2NUM((long)variant.toLongLong());
        }
        case QVariant::ULongLong:
            return UINT2NUM((unsigned long)variant.toULongLong());
        default: {
            kdWarning() << QString("Kross::Ruby::RubyExtension::toVALUE(QVariant) Not possible to convert the QVariant type '%1' to a VALUE.").arg(variant.typeName()) << endl;
            return Qundef;
        }
    }
}

VALUE RubyExtension::toVALUE(Kross::Api::Object::Ptr object)
{
    if(! object) {
        return 0;
    }

    if(object->getClassName() == "Kross::Api::Variant") {
        QVariant v = static_cast<Kross::Api::Variant*>( object.data() )->getValue();
        return toVALUE(v);
    }

    if(object->getClassName() == "Kross::Api::List") {
        Kross::Api::List* list = static_cast<Kross::Api::List*>( object.data() );
        return toVALUE((Kross::Api::List::Ptr)list);
    }

    if(object->getClassName() == "Kross::Api::Dict") {
        Kross::Api::Dict* dict = static_cast<Kross::Api::Dict*>( object.data() );
        return toVALUE((Kross::Api::Dict::Ptr)dict);
    }

    if(RubyExtensionPrivate::s_krossObject == 0)
    {
        RubyExtensionPrivate::s_krossObject = rb_define_class("KrossObject", rb_cObject);
        rb_define_method(RubyExtensionPrivate::s_krossObject, "method_missing",  (VALUE (*)(...))RubyExtension::method_missing, -1);
    }
    return Data_Wrap_Struct(RubyExtensionPrivate::s_krossObject, 0, free, new RubyExtension(object) );
}

VALUE RubyExtension::toVALUE(Kross::Api::List::Ptr list)
{
    VALUE l = rb_ary_new();
    uint count = list ? list->count() : 0;
    for(uint i = 0; i < count; i++)
        rb_ary_push(l, toVALUE(list->item(i)));
    return l;

}

}

}