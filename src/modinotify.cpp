/**
 *  \file modinotify.cpp
 */

#include "modinotify.hpp"


FALCON_MODULE_DECL
{
    #define FALCON_DECLARE_MODULE self

    Falcon::Module* self = new Falcon::Module();
    self->name( "inotify" );
    self->language( "en_US" );
    self->engineVersion( FALCON_VERSION_NUM );
    self->version( VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION );

    #include "modinotify_st.hpp"

    /*
     *  inotify instance
     */

    Falcon::Symbol* c_Instance = self->addClass(
        "InotifyInstance",
        &Falcon::Inotify::Instance::init );

    self->addClassMethod( c_Instance, "add_watch", &Falcon::Inotify::Instance::add_watch );
    self->addClassMethod( c_Instance, "rm_watch", &Falcon::Inotify::Instance::rm_watch );
    self->addClassMethod( c_Instance, "get_event", &Falcon::Inotify::Instance::get_event );

    c_Instance->setWKS( true );
    c_Instance->getClassDef()->factory( &Falcon::Inotify::Instance::factory );


    /*
     *  inotify watcher
     */

    Falcon::Symbol* c_Watcher = self->addClass( "InotifyWatcher" );

    c_Watcher->setWKS( true );
    c_Watcher->getClassDef()->factory( &Falcon::Inotify::Watcher::factory );

    /*
     *  inotify event
     */

    Falcon::Symbol* c_Event = self->addClass( "InotifyEvent" );

    self->addClassMethod( c_Event, "masks", &Falcon::Inotify::Event::masks );

    c_Event->setWKS( true );
    c_Event->getClassDef()->factory( &Falcon::Inotify::Event::factory );

    /*
     *  constants
     */

    self->addConstant( "IN_NONBLOCK",   (Falcon::int64) IN_NONBLOCK );
    self->addConstant( "IN_CLOEXEC",    (Falcon::int64) IN_CLOEXEC );

    self->addConstant( "IN_ACCESS",     (Falcon::int64) IN_ACCESS );
    self->addConstant( "IN_ATTRIB",     (Falcon::int64) IN_ATTRIB );
    self->addConstant( "IN_CLOSE_WRITE",(Falcon::int64) IN_CLOSE_WRITE );
    self->addConstant( "IN_CLOSE_NOWRITE",(Falcon::int64) IN_CLOSE_NOWRITE );
    self->addConstant( "IN_CREATE",     (Falcon::int64) IN_CREATE );
    self->addConstant( "IN_DELETE",     (Falcon::int64) IN_DELETE );
    self->addConstant( "IN_DELETE_SELF",(Falcon::int64) IN_DELETE_SELF );
    self->addConstant( "IN_MODIFY",     (Falcon::int64) IN_MODIFY );
    self->addConstant( "IN_MOVE_SELF",  (Falcon::int64) IN_MOVE_SELF );
    self->addConstant( "IN_MOVED_FROM", (Falcon::int64) IN_MOVED_FROM );
    self->addConstant( "IN_MOVED_TO",   (Falcon::int64) IN_MOVED_TO );
    self->addConstant( "IN_OPEN",       (Falcon::int64) IN_OPEN );

    self->addConstant( "IN_ALL_EVENTS", (Falcon::int64) IN_ALL_EVENTS );

    return self;
}


namespace Falcon {
namespace Inotify {


Instance::Instance( const Falcon::CoreClass* cls, int fd )
    :
    Falcon::CoreObject( cls )
{
    if ( fd )
        setUserData( (void*) fd );
}


Falcon::CoreObject* Instance::factory( const Falcon::CoreClass* cls, void* _fd, bool )
{
    return new Falcon::Inotify::Instance( cls, (int)_fd );
}


bool Instance::getProperty( const Falcon::String& prop, Falcon::Item& val ) const
{
    if ( prop == "fd" )
        val = (int) getUserData();
    else
        return defaultProperty( prop, val );
    return true;
}


bool Instance::setProperty( const Falcon::String&, const Falcon::Item& )
{
    return false;
}


Falcon::CoreObject* Watcher::factory( const Falcon::CoreClass* cls, void* _fd, bool )
{
    return new Falcon::Inotify::Watcher( cls, (int)_fd );
}


Event::Event( const Falcon::CoreClass* cls, const struct inotify_event *ev )
    :
    Falcon::CoreObject( cls )
{
    if ( ev )
        setUserData( (void*) ev );
}


Event::~Event()
{
    struct inotify_event* m_ev = (struct inotify_event*) getUserData();
    if ( m_ev )
        memFree( m_ev );
}


Falcon::CoreObject* Event::factory( const Falcon::CoreClass* cls, void* ev, bool )
{
    return new Falcon::Inotify::Event( cls, (struct inotify_event*) ev );
}


bool Event::getProperty( const Falcon::String& prop, Falcon::Item& val ) const
{
    struct inotify_event* m_ev = (struct inotify_event*) getUserData();

    if ( prop == "wd" )
        val = m_ev->wd;
    else
    if ( prop == "mask" )
        val = m_ev->mask;
    else
    if ( prop == "cookie" )
        val = m_ev->cookie;
    else
    if ( prop == "len" )
        val = m_ev->len;
    else
    if ( prop == "name" )
    {
        if ( m_ev->len )
            val = new String( m_ev->name );
        else
            val = Item();
    }
    else
        return defaultProperty( prop, val );
    return true;
}


bool Event::setProperty( const Falcon::String&, const Falcon::Item& )
{
    return false;
}


/*#
    @class InotifyInstance
    @brief Initializes a new inotify instance.
    @optparam flags (default 0)

    The following values can be bitwise ORed in flags to obtain different behavior:

    IN_NONBLOCK : Set the O_NONBLOCK file status flag on the new open file description.

    IN_CLOEXEC : Set the close-on-exec (FD_CLOEXEC) flag on the new file descriptor.
    See the description of the O_CLOEXEC flag in open(2) for reasons why this
    may be useful.

 */
FALCON_FUNC Instance::init( Falcon::VMachine* vm )
{
    Falcon::CoreObject* self = vm->self().asObject();

    int flags = 0;

    Item* i_flags = vm->param( 0 );

    if ( i_flags && !i_flags->isNil() )
    {
        if ( !i_flags->isInteger() )
            throw new Falcon::ParamError(
                Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
                .extra( "I" ) );

        flags = i_flags->asInteger();
    }

    int fd = inotify_init1( flags ); // linux >= 2.6.27

    if ( fd == -1 )
    {
        switch ( errno )
        {
        case EINVAL:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EINVAL, __LINE__ )
                .desc( "EINVAL" ) );
        case EMFILE:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EMFILE, __LINE__ )
                .desc( "EMFILE" ) );
        case ENFILE:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_ENFILE, __LINE__ )
                .desc( "ENFILE" ) );
        case ENOMEM:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_ENOMEM, __LINE__ )
                .desc( "ENOMEM" ) );
        default:
            return; // not reached
        }
    }

    self->setUserData( (void*) fd );
}


/*#
    @method add_watch InotifyInstance
    @brief Add a watch
    @param pathname
    @param mask
    @return InotifyWatcher
 */
FALCON_FUNC Instance::add_watch( Falcon::VMachine* vm )
{
    Falcon::CoreObject* self = vm->self().asObject();
    int fd = (int) self->getUserData();

    Item* i_pathname = vm->param( 0 );
    Item* i_mask = vm->param( 1 );

    if ( !i_pathname || !i_pathname->isString()
        || !i_mask || !i_mask->isInteger() )
        throw new Falcon::ParamError(
            Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
            .extra( "S,I" ) );

    AutoCString pathname( *i_pathname );
    int wd = inotify_add_watch( fd, pathname.c_str(), i_mask->asInteger() );

    if ( fd == -1 )
    {
        switch ( errno )
        {
        case EACCES:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EACCES, __LINE__ )
                .desc( "EACCES" ) );
        case EBADF:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EBADF, __LINE__ )
                .desc( "EBADF" ) );
        case EFAULT:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EFAULT, __LINE__ )
                .desc( "EFAULT" ) );
        case EINVAL:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EINVAL, __LINE__ )
                .desc( "EINVAL" ) );
        case ENOMEM:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_ENOMEM, __LINE__ )
                .desc( "ENOMEM" ) );
        case ENOSPC:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_ENOSPC, __LINE__ )
                .desc( "ENOSPC" ) );
        default:
            return; // not reached
        }
    }

    vm->retval( new Falcon::Inotify::Watcher(
        vm->findWKI( "InotifyWatcher" )->asClass(), wd ) );
}


/*#
    @method rm_watch InotifyInstance
    @brief Remove a watch
    @param watcher watcher object
 */
FALCON_FUNC Instance::rm_watch( Falcon::VMachine* vm )
{
    Falcon::CoreObject* self = vm->self().asObject();
    int fd = (int) self->getUserData();

    Item* i_wd = vm->param( 0 );

    if ( !i_wd || !i_wd->isObject()
        || !( i_wd->isOfClass( "InotifyWatcher" )
        || i_wd->isOfClass( "inotify.InotifyWatcher" ) ) )
        throw new Falcon::ParamError(
            Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
            .extra( "InotifyWatcher" ) );

    uint32_t wd = (uint32_t) i_wd->asObject()->getUserData();

    int res = inotify_rm_watch( fd, wd );

    if ( res == -1 )
    {
        switch ( errno )
        {
        case EBADF:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EBADF, __LINE__ )
                .desc( "EBADF" ) );
        case EINVAL:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EINVAL, __LINE__ )
                .desc( "EINVAL" ) );
        default:
            return; // not reached
        }
    }
}


/*#
    @method get_event InotifyInstance
    @brief Get one avent
    @return InotifyEvent
 */
FALCON_FUNC Instance::get_event( Falcon::VMachine* vm )
{
    Falcon::CoreObject* self = vm->self().asObject();
    int fd = (int) self->getUserData();

    struct inotify_event* event = (struct inotify_event*) memAlloc(
        sizeof( struct inotify_event ) + FILENAME_MAX );

    int res = read( fd, event, sizeof( struct inotify_event ) + FILENAME_MAX );

    if ( res == -1 )
    {
        memFree( event );

        switch ( errno )
        {
        case EAGAIN:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EAGAIN, __LINE__ )
                .desc( "EAGAIN" ) );
        case EBADF:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EBADF, __LINE__ )
                .desc( "EBADF" ) );
        case EFAULT:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EFAULT, __LINE__ )
                .desc( "EFAULT" ) );
        case EINTR:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EINTR, __LINE__ )
                .desc( "EINTR" ) );
        case EINVAL:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EINVAL, __LINE__ )
                .desc( "EINVAL" ) );
        case EIO:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EIO, __LINE__ )
                .desc( "EIO" ) );
        case EISDIR:
            throw new InotifyError(
                Falcon::ErrorParam( Falcon::Inotify::e_EISDIR, __LINE__ )
                .desc( "EISDIR" ) );
        default:
            return; // not reached
        }
    }

    vm->retval( new Inotify::Event(
        vm->findWKI( "InotifyEvent" )->asClass(), event ) );
}


/*#
    @method masks
    @brief Check for an event mask
    @param mask (integer)
    @return (boolean) wether the mask matches the event
 */
FALCON_FUNC Event::masks( Falcon::VMachine* vm )
{
    Falcon::CoreObject* self = vm->self().asObject();

    Item* i_mask = vm->param( 0 );

    if ( !i_mask || !i_mask->isInteger() )
        throw new Falcon::ParamError(
            Falcon::ErrorParam( Falcon::e_inv_params, __LINE__ )
            .extra( "I" ) );

    struct inotify_event* m_ev = (struct inotify_event*) self->getUserData();
    vm->retval( m_ev->mask & i_mask->asInteger() );
}


/*#
    @class InotifyError
    @brief Error generated by falcon-inotify
    @optparam code numeric error code
    @optparam description textual description of the error code
    @optparam extra descriptive message explaining the error conditions
    @from Error code, description, extra

    See the Error class in the core module.
*/
FALCON_FUNC InotifyError_init( Falcon::VMachine* vm )
{
    Falcon::CoreObject* self = vm->self().asObject();

    if ( !self->getUserData() )
        self->setUserData( new InotifyError );

    Falcon::core::Error_init( vm );
}


} // Inotify
} // Falcon
