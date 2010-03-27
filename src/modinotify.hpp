#ifndef MODINOTIFY_HPP
#define MODINOTIFY_HPP

#include <falcon/autocstring.h>
#include <falcon/coreobject.h>
#include <falcon/error.h>
#include <falcon/falcondata.h>
#include <falcon/item.h>
#include <falcon/module.h>
#include <falcon/vm.h>

#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>

#include "modinotify_st.hpp"
#include "modinotify_version.hpp"


namespace Falcon {

/**
 *  \namespace Falcon::Inotify
 */
namespace Inotify {


/**
 *  \class Falcon::Inotify::Instance
 */
class Instance
    :
    public Falcon::CoreObject
{
public:

    Instance( const Falcon::CoreClass*, int );

    ~Instance() {}

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

    Falcon::CoreObject* clone() const { return 0; }

    bool getProperty( const Falcon::String&, Falcon::Item& ) const;

    bool setProperty( const Falcon::String&, const Falcon::Item& );

    /**
     *  \brief create an inotify instance
     */
    static FALCON_FUNC init( Falcon::VMachine* );


    /**
     *  \brief create a watcher
     */
    static FALCON_FUNC add_watch( Falcon::VMachine* );


    /**
     *  \brief remove a watcher
     */
    static FALCON_FUNC rm_watch( Falcon::VMachine* );


    /**
     *  \brief get one event
     */
    static FALCON_FUNC get_event( Falcon::VMachine* );

};


/**
 *  \class Falcon::Inotify::Watcher
 */
class Watcher
    :
    public Falcon::Inotify::Instance
{
public:

    Watcher( const Falcon::CoreClass* cls, int fd )
        :
        Instance( cls, fd )
    {}

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

};


/**
 *  \class Falcon::Inotify::Event
 */
class Event
    :
    public Falcon::CoreObject
{
public:

    Event( const Falcon::CoreClass*, const struct inotify_event * );

    ~Event();

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

    Falcon::CoreObject* clone() const { return 0; }

    bool getProperty( const Falcon::String&, Falcon::Item& ) const;

    bool setProperty( const Falcon::String&, const Falcon::Item& );

    /**
     *  \brief test for a mask of events
     */
    static FALCON_FUNC masks( Falcon::VMachine* );

};


/**
 *  \class Falcon::Inotify::InotifyError
 */
class InotifyError
    :
    public Falcon::Error
{
public:

    InotifyError()
        :
        Error( "InotifyError" )
    {}

    InotifyError( const ErrorParam& params  )
        :
        Error( "InotifyError", params )
    {}

};


/**
 *  \brief exception type initialization
 */
FALCON_FUNC InotifyError_init ( Falcon::VMachine* );


/**
 *  \enum Falcon::Inotify::InotifyErrorIds
 */
enum InotifyErrorIds
{
    e_EINVAL    = EINVAL,
    e_EMFILE    = EMFILE,
    e_ENFILE    = ENFILE,
    e_ENOMEM    = ENOMEM,
    e_EACCES    = EACCES,
    e_EBADF     = EBADF,
    e_EFAULT    = EFAULT,
    e_ENOSPC    = ENOSPC,
    e_EAGAIN    = EAGAIN,
    e_EINTR     = EINTR,
    e_EIO       = EIO,
    e_EISDIR    = EISDIR
};


} // Inotify
} // Falcon

#endif // !MODINOTIFY_HPP
