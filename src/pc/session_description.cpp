// impl session_description.h
#include "pc/session_description.h"
#include <sstream>
namespace grtc
{
    bool ContentGroup::has_content_name(const std::string& content_name){
        for(auto name : _content_names){
            if(name == content_name){
                return true;
            }
        }
        return false;
    }

    void ContentGroup::add_content_name(const std::string& content_name){
        if(!has_content_name(content_name)){
            _content_names.push_back(content_name);
        }
    }

    SessionDescription::SessionDescription(SdpType type) : _sdp_type(type)
    {
    }

    SessionDescription::~SessionDescription()
    {
    }

    void SessionDescription::add_group(const ContentGroup& group){
        _content_groups.push_back(group);
    }
    void SessionDescription::add_content(std::shared_ptr<MediaContentDescription> content)
    {
        _contents.push_back(content);
    }

    std::vector<const ContentGroup*>  SessionDescription::get_group_by_name(const std::string& name) const{
        std::vector<const ContentGroup*> content_group;
        for(const ContentGroup& group : _content_groups){
            if(group.semantics() == name){
                content_group.push_back(&group);
            }
        }
        return content_group;
    }

    std::string SessionDescription::to_string()
    {
        std::stringstream ss;
        ss << "v=0\r\n";
        // session origin
        // RFC 4566
        // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
        ss << "o=- 0 2 IN IP4 127.0.0.1\r\n";
        // session name
        ss << "s=-\r\n";
        // time description
        ss << "t=0 0\r\n";

        //BUNDLE 信息
        std::vector<const ContentGroup*> content_group = get_group_by_name("BUNDLE");
        if(!content_group.empty()){
            ss << "a=group:BUNDLE";
            for(auto group : content_group){
                for(auto content_name : group->content_names()){
                    ss << " " << content_name;
                }
            }
            ss << "\r\n";
        }


        return ss.str();
    }

} // namespace grtc
