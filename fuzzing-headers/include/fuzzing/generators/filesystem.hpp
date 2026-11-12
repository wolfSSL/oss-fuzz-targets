#pragma once

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/datasource/id.hpp>
#include <fuzzing/exception.hpp>

#include <cstdio>
#include <dirent.h>
#include <memory>
#include <optional>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


namespace fuzzing {
namespace generators {
namespace filesystem {

using global_FlowException = exception::FlowException;
class FlowException : public global_FlowException {
    public:
        FlowException() : global_FlowException() { }
};

static const std::string generateFilename(datasource::Datasource& ds) {
    auto filename = ds.Get<std::string>();
    if ( filename.empty() ) {
        throw FlowException();
    }
    for (size_t i = 0; i < filename.size(); i++) {
#if 0
        if ( filename[i] == '.' && i + 1 < filename.size() && filename[i+1] == '.' ) {
            filename[i] = '_';
        } else if ( filename[i] == '~' ) {
            filename[i] = '_';
        } else if ( filename[i] == '/' ) {
            /* TODO allowed if preceded by backspace */
            filename[i] = '_';
        }
#endif
        if ( !isdigit(filename[i]) ) {
            filename[i] = '_';
        }
    }

    return filename;
}

static bool pathExists(const std::string fullPath) {
    struct stat st;
    /* Necessary? */
    memset(&st, 0, sizeof(st));

    return stat(fullPath.c_str(), &st) == -1 ? false : true; 
}

static std::optional<struct stat> getStat(const std::string fullPath) {
    struct stat st;
    /* Necessary? */
    memset(&st, 0, sizeof(st));

    if ( stat(fullPath.c_str(), &st) == -1 ) {
        return std::nullopt;
    } else {
        return st; 
    }

}

class FileAttributes {
    private:
        const std::string name;
        /* TODO date etc */
    public:
        FileAttributes(datasource::Datasource ds) :
            name(generateFilename(ds))
        { }

#if 0
        FileAttributes(const std::string fullPath) {
            /* TODO split fullPath, get last part */
            const auto st = getStat(fullPath);

            if ( st == std::nullopt ) {
                /* TODO */
                abort();
            }
            
            /* TODO fill in attributes from st */
        }
#endif

        std::string Name(void) const {
            return name;
        }
};

class AbstractFile {
    protected:
        const FileAttributes attributes;
        const std::string basePath;
        const std::string getFullPath(void) const {
            return basePath + "/" + Name();
        }

    public:
        AbstractFile(datasource::Datasource ds, const std::string basePath) :
        attributes(ds),
        basePath(basePath)
        { }

        std::string Name(void) const {
            return attributes.Name();
        }

        virtual ~AbstractFile() { }

        virtual bool Write(void) const = 0;
        virtual bool Verify(void) const = 0;
        virtual bool Remove(void) const = 0;
        virtual std::string ToString(void) const = 0;
};

class File : public AbstractFile {
    private:
        const std::vector<uint8_t> content;

    public:
        File(datasource::Datasource& ds, const std::string basePath) :
        AbstractFile(ds, basePath),
        content( ds.GetVector<uint8_t>() )
        { }

        bool Write(void) const override {
            /* Should not exist */
            if ( pathExists(getFullPath()) == true ) {
                return false;
            }

            bool ret = false;

            /* Create */
            FILE* fp = fopen(getFullPath().c_str(), "wb");
            if ( fp == nullptr ) {
                return false;
            }

            if ( content.empty() ) {
                /* Skip write step */
                ret = true;
                goto end;
            }

            /* Write */
            if ( fwrite(content.data(), content.size(), 1, fp) != 1 ) {
                goto end;
            }

            ret = true;

end:
            /* Close */
            fclose(fp);

            return ret;
        }

        bool Verify(void) const override {
            const auto st = getStat(getFullPath());

            /* Should exist */
            if ( st == std::nullopt ) {
                return false;
            }

            /* Should be regular file */
            if ( !(S_ISREG(st->st_mode)) ) {
                return false;
            }

            /* Must do this before casting to unsigned (below) */
            if ( st->st_size < 0 ) {
                return false;
            }

            /* Size should match */
            if ( static_cast<size_t>(st->st_size) != content.size() ) {
                return false;
            }
            
            std::vector<uint8_t> content_copy;
            content_copy.resize( content.size() );

            bool ret = false;

            /* Open */
            FILE* fp = fopen(getFullPath().c_str(), "rb");
            if ( fp == nullptr ) {
                return false;
            }

            if ( content.empty() ) {
                /* Skip read step */
                ret = true;
                goto end;
            }

            /* Read */
            if ( fread(content_copy.data(), content.size(), 1, fp) != 1 ) {
                goto end;
            }

            /* Content should match */
            if ( content_copy != content ) {
                goto end;
            }

            ret = true;

end:
            /* Close */
            fclose(fp);

            return ret;
        }

        bool Remove(void) const override {
            /* Should exist */
            if ( pathExists(getFullPath()) == false ) {
                return false;
            }

            /* Remove */
            return remove(getFullPath().c_str()) == 0 ? true : false;
        }

        std::string ToString(void) const override {
            return getFullPath() + "\n";
        }
};

class Directory : public AbstractFile {
    private:
        std::vector<std::shared_ptr<AbstractFile>> members;

        bool isDuplicate(const std::string& name) const {

            for (const auto& member : members) {
                if ( member->Name() == name ) {
                    return true;
                }
            }

            return false;
        }

        bool matchDirectoryListing(void) const {

            std::set<std::string> fsFilenames;
            std::set<std::string> memberFilenames;

            /* Get list of names of files in directory */
            {
                DIR* d = opendir(getFullPath().c_str());
                if ( d == nullptr ) {
                    return false;
                }

                struct dirent *dir;
                while ((dir = readdir(d)) != nullptr) {
                    const std::string toAdd = dir->d_name;
                    if ( toAdd == "." || toAdd == "..") {
                        continue;
                    }

                    fsFilenames.insert( std::move(toAdd) );
                }

                closedir(d);
            }

            /* Get list of names of files in 'members' */
            for (const auto& member : members) {
                memberFilenames.insert( member->Name() );
            }

            /* There should be a perfect overlap */
            if ( fsFilenames != memberFilenames ) {
                return false;
            }

            return true;
        }

    public:
        Directory(datasource::Datasource& ds, const std::string basePath, const int depth = 0) :
        AbstractFile(ds, basePath)
        {
            while ( ds.Get<bool>() == true ) {
                std::shared_ptr<AbstractFile> newFile;

                if ( ds.Get<bool>() == true ) {
                    newFile = std::make_shared<File>(ds, getFullPath());
                } else {
                    if ( depth + 1 > 4096 ) {
                        throw FlowException();
                    }
                    newFile = std::make_shared<Directory>(ds, getFullPath(), depth+1);
                }

                if ( isDuplicate(newFile->Name()) ) {
                    throw FlowException();
                }

                members.emplace_back( std::move(newFile) );
            } 
        }

        bool Write(void) const override {
            /* Should not exist */
            if ( pathExists(getFullPath()) == true ) {
                return false;
            }

            /* Create */
            if ( mkdir(getFullPath().c_str(), 0700 /* TODO permissions from FileAttributes */) != 0 ) {
                return false;
            }

            /* Write all children recursively */
            for (const auto& member : members) {
                if ( member->Write() == false ) {
                    return false;
                }
            } 

            /* Success */
            return true;
        }

        bool Verify(void) const override {
            const auto st = getStat(getFullPath());

            /* Should exist */
            if ( st == std::nullopt ) {
                return false;
            }

            /* Should be a directory */
            if ( !(S_ISDIR(st->st_mode)) ) {
                return false;
            }

            /* Verify all children recursively */
            for (const auto& member : members) {
                if ( member->Verify() == false ) {
                    return false;
                }
            } 

            if ( matchDirectoryListing() == false ) {
                return false;
            }

            /* Success */
            return true;
        }

        bool Remove(void) const override {
            /* Should exist */
            if ( pathExists(getFullPath()) == false ) {
                return false;
            }

            /* Remove all children recursively */
            for (const auto& member : members) {
                if ( member->Remove() == false ) {
                    return false;
                }
            } 

            /* Remove */
            if ( remove(getFullPath().c_str()) != 0 ) {
                return false;
            }

            /* Success */
            return true;
        }

        std::string ToString(void) const override {
            std::string ret;

            ret += getFullPath() + "\n";

            for (const auto& member : members) {
                ret += member->ToString();
            }

            return ret;
        }
};

class Filesystem {
    private:
        std::shared_ptr<AbstractFile> fsRoot;
    public:
        Filesystem(datasource::Datasource& ds, const std::string fsRootPath) :
            fsRoot( std::make_shared<Directory>(ds, fsRootPath) )
        { }

        bool Write(void) const {
            return fsRoot->Write();
        }

        bool Verify(void) const {
            return fsRoot->Verify();
        }

        bool Remove(void) const {
            return fsRoot->Remove();
            /* TODO verify that fsRootPath is now empty */
        }

        std::string ToString(void) const {
            return fsRoot->ToString();
        }
};

} /* namespace filesystem */
} /* namespace generators */
} /* namespace fuzzing */
