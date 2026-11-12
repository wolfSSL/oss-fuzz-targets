#pragma once

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/datasource/id.hpp>
#include <fuzzing/generators/filesystem.hpp>
#include <fuzzing/exception.hpp>
#include <fuzzing/util/binaryexecutor.hpp>
#include <stdlib.h>

namespace fuzzing {
namespace testers {
namespace filesystem {

class FilesystemTester {
    protected:
        datasource::Datasource& ds;
        const std::string fsRootPath;
        virtual bool transform(void) = 0;
    private:
        const fuzzing::generators::filesystem::Filesystem fs;
    public:
        FilesystemTester(datasource::Datasource& ds, const std::string fsRootPath) :
            ds(ds), fsRootPath(fsRootPath), fs(ds, fsRootPath)
        { }

        bool Run(void) {
            if ( fs.Write() == false ) {
                /* Write can fail due to overlong/invalid paths.
                 * Not a critical error
                 */
                return false;
            }

            if ( fs.Verify() == false ) {
                throw exception::TargetException(
                    "Writing succeeded, but verifying the written data failed"
                );
                return false;
            }

            if ( transform() == false ) {
                return false;
            }

            if ( fs.Verify() == false ) {
                throw exception::TargetException(
                    "Verifying data failed after transformation"
                );
                return false;
            }

            if ( fs.Remove() == false ) {
                throw exception::TargetException(
                    "Removing the data failed"
                );
                return false;
            }

            return true;
        }
};

class ArchiverTester : public FilesystemTester {
    protected:
        virtual bool pack(const std::string infile, const std::string outfile) = 0;
        virtual bool unpack(const std::string infile) = 0;
    private:
        bool transform(void) {
            if ( pack(fsRootPath + "/", "archive") == false ) {
                return false;
            }

            {
                static const auto rmdirCmd = std::string("rm -rf " + fsRootPath + "/");
                if ( system(rmdirCmd.c_str()) != 0 ) {
                    throw exception::TargetException(
                        "Removing the working directory failed"
                    );
                }
            }

            if ( unpack("archive") == false ) {
                throw exception::TargetException(
                    "Tar cannot process its own data"
                );
                return false;
            }

            {
                static const auto rmTarCmd = std::string("rm -rf archive.tar");
                if ( system(rmTarCmd.c_str()) != 0 ) {
                    throw exception::TargetException(
                        "Removing the archive failed"
                    );
                }
            }

            return true;
        }
    public:
        ArchiverTester(fuzzing::datasource::Datasource& ds, const std::string fsRootPath) :
            FilesystemTester(ds, fsRootPath)
        { }
};

template <typename BinaryExecutor>
class TarTester : public ArchiverTester {
    static_assert(std::is_base_of<util::BinaryExecutor, BinaryExecutor>::value);
    private:
        const std::string tarCmd;
        std::string extraFlags;

        bool pack(const std::string infile, const std::string outfile) override {
            const auto cmd = std::string(tarCmd + " " + extraFlags + " --create --file " + outfile + " " + infile);

            BinaryExecutor executor(cmd);
            return executor.Run();
        }

        bool unpack(const std::string infile) override {
            const auto cmd = std::string(tarCmd + " " + extraFlags + " --extract --file " + infile);

            BinaryExecutor executor(cmd);
            return executor.Run();
        }
    public:
        TarTester(
                fuzzing::datasource::Datasource& ds,
                const std::string fsRootPath,
                const std::string tarCmd) :
            ArchiverTester(ds, fsRootPath),
            tarCmd(tarCmd.empty() ? "tar" : tarCmd)
        {
            const auto compression = ds.Get<uint8_t>();

            switch ( compression ) {
                case    1:
                    extraFlags += "--gzip ";
                    break;
                case    2:
                    extraFlags += "--bzip2 ";
                    break;
                case    3:
                    extraFlags += "--xz ";
                    break;
                case    4:
                    extraFlags += "--lzip ";
                    break;
                case    5:
                    extraFlags += "--lzma ";
                    break;
                case    6:
                    extraFlags += "--lzop ";
                    break;
            }

            {
                std::string sortArg = "--sort=";

                const auto sort = ds.Get<uint8_t>();

                switch ( sort ) {
                    case    1:
                        sortArg += "name";
                        break;
                    case    2:
                        sortArg += "inode";
                        break;
                    default:
                        sortArg += "none";
                        break;
                }

                sortArg += " ";

                extraFlags += sortArg;
            }

            if ( ds.Get<bool>() == true ) {
                extraFlags += "--seek ";
            }

            {
                std::string formatArg = "--format=";

                const auto format = ds.Get<uint8_t>();

                switch ( format ) {
                    case    1:
                        formatArg += "oldgnu";
                        break;
                    case    2:
                        formatArg += "pax";
                        break;
                    case    3:
                        formatArg += "ustar";
                        break;
                    case    4:
                        formatArg += "v7";
                        break;
                    default:
                        formatArg += "gnu";
                        break;
                }

                formatArg += " ";

                extraFlags += formatArg;
            }
        }
};

} /* namespace filesystem */
} /* namespace testers */
} /* namespace fuzzing */
