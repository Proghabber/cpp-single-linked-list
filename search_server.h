//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎
#pragma once

#include <vector>
#include <string>
using namespace std;


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double  DEVIATION = 1e-6;


vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}


struct Document {

    Document() 
    :id(0), relevance(0.0), rating(0)
     {
     }

     Document(int id, double relevance, int rating) 
     :id(id), relevance(relevance), rating(rating)
     {
     }

    int id;
    double relevance;
    int rating;
};


enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};


class SearchServer {
public:
    inline static constexpr int INVALID_DOCUMENT_ID = -1;
    template<typename list>
    SearchServer(list setter) {
        SetStopWords(setter);
    }

    void SetStopWords(const string& text) {
        set <string> set_words;
        for (const string& word : SplitIntoWords(text)){
            set_words.insert(word);
        }
        SetStopWords(set_words);
    }


template<typename list>
    void SetStopWords(const list& set_words) {
        for (const string& word : set_words) {
            if (!word.empty()){
                if (!NotSpecSymbol(word)){
                    throw invalid_argument("Присутствие спец символов недопустимо"s);
            }
                stop_words_.insert(word);
        }
        }
    }



    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        if (document_id < 0){
            throw invalid_argument("ID документа меньше 0"s);
        } else if (documents_.count(document_id)){
            throw invalid_argument("Документ с таким id уже есть в базе"s);
        }

        const vector<string> words = SplitIntoWordsNoStop(document);
        for (const string& word : words){
            if (!NotSpecSymbol(word)){
                 throw invalid_argument("Документ меет запрещенные символы"s);
            }
        }
        set_id_.push_back(document_id);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words){
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }
    
    
    template <typename compor>
    vector<Document> FindTopDocuments(const string& raw_query,
                                        compor filter_func) const {
        vector<Document> result;                                                         
        const Query query = ParseQuery(raw_query);
    
        result = FindAllDocuments(query, filter_func);
         
        sort(result.begin(), result.end(),
             [](const Document& lhs, const Document& rhs){
                 if (abs(lhs.relevance - rhs.relevance) < DEVIATION){
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (result.size() > MAX_RESULT_DOCUMENT_COUNT){
            result.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return result;
    }
 
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus match_status) const {
       return  FindTopDocuments(raw_query,[match_status](int document_id, DocumentStatus status, int rating){return match_status==status;});
    }

    vector<Document> FindTopDocuments(const string& raw_query) const { 
       return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                        int document_id)  const{
        
        const Query query = ParseQuery(raw_query);
        

        vector<string> matched_words;
        for (const string& word : query.plus_words){
            if (word_to_document_freqs_.count(word) == 0){
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)){
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words){
            if (word_to_document_freqs_.count(word) == 0){
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)){
                matched_words.clear();
                break;
            }
        }
        tuple<vector<string>, DocumentStatus> result ={matched_words, documents_.at(document_id).status};
        return result;
    }

    int GetDocumentId(int index) const {
        if (index < 0 || index > set_id_.size()){
            throw out_of_range( "index за границами размера списка документов "s);
            return INVALID_DOCUMENT_ID;
        }
        return set_id_[index];
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> set_id_;
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    static bool NotSpecSymbol(const string& word){
         
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }
        
    
     

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)){
            if (!IsStopWord(word)){
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()){
            return 0;
        }
        int size=static_cast<int>(ratings.size());
        return accumulate(begin(ratings), end(ratings), 0, plus<int>()) / size;       
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        string mistake;
        // Word shouldn't be empty
        if (text[0] == '-'){
            is_minus = true;
            text = text.substr(1);
        }
        if (text.empty()){         
            throw invalid_argument( "index за границами размера списка документов "s);
        }
        if (text[0] == '-'){
            throw invalid_argument( "два минуса у минус слова "s);
        } 
        if (!NotSpecSymbol(text)){
            throw invalid_argument( "минус слово имеет недопустимые символы"s);
        }
        
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)){
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus){
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    
    template <typename compor>
    vector<Document> FindAllDocuments(const Query& query, compor func) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words){
            if (word_to_document_freqs_.count(word) == 0){
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)){
                if (func(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)){
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words){
            if (word_to_document_freqs_.count(word) == 0){
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)){
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance){
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
      
};