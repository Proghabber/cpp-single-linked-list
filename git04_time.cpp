
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

#include "string_processing.h"
#include "search_server.h"
using namespace std;




string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}










template <typename Iterator>
class Paginator {
        // тело класса
    public:
    Paginator(Iterator begin, Iterator end, int page_size)
        :begin_(begin), end_(end), pages(page_size)
    {
       SetStep();
       FullList();
    
    }

    vector<vector<typename Iterator::value_type>> ReturnList(){
        return list;
    }

    
    private:

    void FullList(){
        for(auto iter = begin_; iter != end_; iter += step){
            Iterator start = iter;
            Iterator last;
            if ((end_ - start) >= step){
                last = start + step;
                list.push_back({start, last});
            } else {
                last = end_;
                list.push_back({start, last});
                break;
            }

            
        }
    }

    void SetStep(){
        if (pages <= 0){
            throw domain_error("вы ввели ноль страниц некуда вывести информацию !"s);
        } else if (pages <= end_ - begin_){
            if ((end_ - begin_) % pages != 0){
                step = (end_ - begin_) / pages + 1;
            } else {
                step = (end_ - begin_) / pages;
            }
        } else if (pages > end_ - begin_ && end_ != begin_){
                step = 1;
        } else {
            step = 0;
        }

    }
    Iterator begin_;
    Iterator end_;
    int pages;
    int step;
    vector<vector<typename Iterator::value_type>> list;
    
}; 


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



template<typename T>
ostream& operator<<(ostream& out, vector<T> vec){
    for (T elem : vec){
        out<<elem;
    }
    return out;
}

ostream& operator<<(ostream& out, const Document& document){
    out << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s ;
    return out;
}
// ==================== для примера =========================


template<typename T>
vector<vector<T>>Paginate(vector<T> list, int page_size){
    Paginator exem(list.begin(), list.end(), page_size);
    return exem.ReturnList();
}

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