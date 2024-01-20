
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <exception>
#include <deque>

//#include "string_processing.h"
#include "search_server.h"
#include "read_input_functions.h"
#include "paginator.h"
using namespace std;

















class RequestQueue {
public:
    explicit RequestQueue( const SearchServer& search_server) 
    :server_(search_server)
    {
        // напишите реализацию
    }
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
        // напишите реализацию
       vector<Document> list_document;
        QueryResult result;
        bool empty=false;
        list_document=server_.FindTopDocuments(raw_query,document_predicate);
         if(list_document.empty()){
              empty=true;  
              empty_reqest_++;   
        }
        result=SetQuery(empty, raw_query,"comporator"s);
        AddIlem(result);

        return list_document;
    }
    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status) {
        // напишите реализацию
        
        vector<Document> list_document;
        QueryResult result;
        bool empty=false;
        list_document=server_.FindTopDocuments(raw_query,status);
        if(list_document.empty()){
              empty=true;  
              empty_reqest_++;   
        }
        result=SetQuery(empty, raw_query, "status"s);
        AddIlem(result);

        return list_document;
    }
    vector<Document> AddFindRequest(const string& raw_query) {
        // напишите реализацию       
        vector<Document> list_document;
        QueryResult result;
        bool empty=false;
        list_document=server_.FindTopDocuments(raw_query);
        if(list_document.empty()){
              empty=true;
              empty_reqest_++;   
        }
        result=SetQuery(empty, raw_query,"empty"s);
        AddIlem(result);

        return list_document;
    }
    int GetNoResultRequests() const {
        // напишите реализацию
       return empty_reqest_;
    }
private:
    struct QueryResult {
        // определите, что должно быть в структуре
        bool empty ;
        string raw_query;
        string filter;
        
        
    };
    deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int empty_reqest_ = 0;
    
    // возможно, здесь вам понадобится что-то ещё
    const SearchServer &server_;

    QueryResult SetQuery(bool empty, string raw_query, string filte){
        QueryResult ret={empty, raw_query, filte};
        return ret;
    }

    void AddIlem(QueryResult& elem){
        requests_.push_back(elem);
        if(requests_.size() > min_in_day_){
            if(requests_.front().empty || empty_reqest_ > 0){
                empty_reqest_--;
            }
            requests_.pop_front();
        }
    }
}; 





int main() {
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server);
    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});
    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
        
    }
    // все еще 1439 запросов с нулевым результатом
    cout<<request_queue.GetNoResultRequests()<<" ";
    request_queue.AddFindRequest("curly dog"s);
    cout<<request_queue.GetNoResultRequests()<<endl;
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    cout<<request_queue.GetNoResultRequests()<<" ";
    request_queue.AddFindRequest("big collar"s);
    cout<<request_queue.GetNoResultRequests()<<endl;
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("sparrow"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    return 0;
} 