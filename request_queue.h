//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».
#pragma once

#include <vector>
#include <deque>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue( const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
       std::vector<Document> list_document;
        QueryResult result;
        bool empty=false;
        list_document=server_.FindTopDocuments(raw_query,document_predicate);
         if(list_document.empty()){
              empty=true;  
              empty_reqest_++;   
        }
        result=SetQuery(empty, raw_query,"comporator");
        UpdateQuery(result);

        return list_document;
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
      
private:
    struct QueryResult {
        bool empty ;
        std::string raw_query;
        std::string filter;
        
        
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int empty_reqest_ = 0;
    const SearchServer &server_;

    QueryResult SetQuery(bool empty, std::string raw_query, std::string filte);

    void UpdateQuery(QueryResult& elem);
}; 