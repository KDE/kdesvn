namespace svn {
    class Version {

    public:
        Version(){}
        ~Version(){}

        static bool client_version_compatible();
        static const char*linked_version();
        static const char*running_version();
    };
}
