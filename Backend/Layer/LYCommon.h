#pragma once

namespace Layer {
    /* We need to establish some relation between all the objects in order to store them in a single container. One way
     * is to use a common non template base class
    */
    class NonTemplateBase {
        public:
            /* The destructor is virtual for the base class because if you did not have a virtual destructor and through
             * the pointer to base class when you call destructor you end up calling base class destructor. In this case
             * you want polymorphism to work on your destructor as well, for example, through calling destructor on your
             * base class you want to end up calling destructor of your most derived class not JUST your base class
            */
            virtual ~NonTemplateBase (void) = 0;
    };
    /* Pure virtual destructors must be defined, which is against the pure virtual behaviour. The only difference between
     * virtual and pure virtual destructor is, that pure virtual destructor will make its base class abstract, hence you
     * cannot create object of that class (hence why we are doing it). We need an implementation here because if you
     * derive anything from base (upcasting) and then try to delete or destroy it, base's destructor will eventually be
     * called. Since it is pure and doesn't have an implementation, will cause compilation error
    */
    inline NonTemplateBase::~NonTemplateBase (void) {}
}   // namespace Layer