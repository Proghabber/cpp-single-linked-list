//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».
#include "request_queue.h"

RequestQueue::RequestQueue( const SearchServer& search_server) 
    :server_(search_server)
    {
    }

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        
        std::vector<Document> list_document;
        QueryResult result;
        bool empty=false;
        list_document=server_.FindTopDocuments(raw_query,status);
        if(list_document.empty()){
              empty=true;  
              empty_reqest_++;   
        }
        result=SetQuery(empty, raw_query, "status");
        UpdateQuery(result);

        return list_document;
    }

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
              
        std::vector<Document> list_document;
        QueryResult result;
        bool empty=false;
        list_document=server_.FindTopDocuments(raw_query);
        if(list_document.empty()){
              empty=true;
              empty_reqest_++;   
        }
        result=SetQuery(empty, raw_query,"empty");
        UpdateQuery(result);

        return list_document;
    }
int RequestQueue::GetNoResultRequests() const {
       return empty_reqest_;
    }

  RequestQueue::QueryResult RequestQueue::SetQuery(bool empty, std::string raw_query, std::string filte){
        QueryResult ret={empty, raw_query, filte};
        return ret;
    }
void RequestQueue::UpdateQuery(QueryResult& elem){
        requests_.push_back(elem);
        if(requests_.size() > min_in_day_){
            if(requests_.front().empty || empty_reqest_ > 0){
                empty_reqest_--;
            }
            requests_.pop_front();
        }
    }