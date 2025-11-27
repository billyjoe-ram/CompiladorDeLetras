#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>
#include <sstream>

using namespace std;

static constexpr long long PESO_VCCV = 100000LL;
static constexpr long long PESO_VVV  = 90000LL;
static constexpr long long PESO_CCV  = 50000LL;
static constexpr long long PESO_CV   = 40000LL;
static constexpr long long PESO_VC   = 30000LL;
static constexpr long long PESO_C    = 10LL;
static constexpr long long PESO_V    = 5LL;

struct Simbolo {
    char caractere;
    bool vogal;
    size_t pos;
    Simbolo() = default;
    explicit Simbolo(char c, bool v, size_t p) : caractere(c), vogal(v), pos(p) {}
};

inline bool eh_vogal_char(char c) {
    const auto x = static_cast<unsigned char>(c);
    switch (std::tolower(x)) {
        case 'a': case 'e': case 'i': case 'o': case 'u':
            return true;
        default:
            return false;
    }
}

[[nodiscard]] static std::string trim(const std::string &s) {
    size_t b = 0;
    while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    if (b == s.size()) return {};
    size_t e = s.size() - 1;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e]))) --e;
    return s.substr(b, e - b + 1);
}

[[nodiscard]] static std::vector<Simbolo> tokenizar(const std::string &cadeia, bool &ok, std::string &erro) {
    std::vector<Simbolo> tokens;
    ok = true;
    erro.clear();
    const size_t n = cadeia.size();
    tokens.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        const auto a = static_cast<unsigned char>(cadeia[i]);
        if (!std::isalpha(a)) {
            ok = false;
            erro = "caractere nao alfabetico na posicao " + std::to_string(i);
            return {};
        }
        const bool v = eh_vogal_char(cadeia[i]);
        tokens.emplace_back(cadeia[i], v, i);
    }
    return tokens;
}

[[nodiscard]] static bool validar_cadeia_globais(const std::string &cadeia, std::string &motivo, size_t &pos_erro) {
    motivo.clear();
    pos_erro = 0;
    const size_t n = cadeia.size();
    if (n == 0) {
        motivo = "entrada vazia";
        return false;
    }
    auto eh_v = [](char c) { return eh_vogal_char(c); };
    size_t run_v = 0;
    for (size_t i = 0; i < n; ++i) {
        if (eh_v(cadeia[i])) {
            ++run_v;
            if (run_v > 3) {
                motivo = "mais de 3 vogais consecutivas";
                pos_erro = i;
                return false;
            }
        } else {
            run_v = 0;
        }
    }
    if (!eh_v(cadeia[n - 1])) {
        motivo = "cadeia nao termina em vogal";
        pos_erro = n - 1;
        return false;
    }
    size_t p = 0;
    while (p < n) {
        if (!eh_v(cadeia[p])) {
            size_t q = p;
            while (q < n && !eh_v(cadeia[q])) ++q;
            const size_t run_c = q - p;
            if (run_c >= 2) {
                bool ok_run = false;
                if (p == 0) {
                    if (run_c <= 2 && q < n && eh_v(cadeia[q])) ok_run = true;
                } else if (q < n) {
                    if (eh_v(cadeia[p - 1]) && eh_v(cadeia[q])) ok_run = true;
                }
                if (!ok_run) {
                    motivo = "sequencia de consoantes invalida";
                    pos_erro = p;
                    return false;
                }
            }
            p = q;
        } else {
            ++p;
        }
    }
    return true;
}

struct NoAST {
    enum class Tipo { Palavra, Silaba } tipo;
    std::string texto;
    size_t pos_inicio;
    NoAST() noexcept : tipo(Tipo::Palavra), texto(), pos_inicio(0) {}
    static NoAST faz_silaba(std::string t, size_t p) {
        NoAST n;
        n.tipo = Tipo::Silaba;
        n.texto = std::move(t);
        n.pos_inicio = p;
        return n;
    }
    static NoAST faz_palavra() {
        NoAST n;
        n.tipo = Tipo::Palavra;
        n.texto.clear();
        n.pos_inicio = 0;
        return n;
    }
};

struct Parser {
    const std::vector<Simbolo> &tokens;
    size_t n;
    std::vector<long long> dp;
    std::vector<size_t> prox;
    static constexpr long long NEG = std::numeric_limits<long long>::min() / 4;

    explicit Parser(const std::vector<Simbolo> &t)
            : tokens(t), n(t.size()), dp(n + 1, NEG), prox(n + 1, std::numeric_limits<size_t>::max()) {}

    [[nodiscard]] std::vector<std::pair<size_t,long long>> candidatos(size_t i) const {
        std::vector<std::pair<size_t,long long>> r;
        if (i >= n) return r;
        auto v = [&](size_t k){ return tokens[k].vogal; };
        if (i + 3 < n && v(i) && !v(i+1) && !v(i+2) && v(i+3)) r.emplace_back(4, PESO_VCCV);
        if (i + 2 < n && v(i) && v(i+1) && v(i+2)) r.emplace_back(3, PESO_VVV);
        if (i + 2 < n && !v(i) && !v(i+1) && v(i+2)) r.emplace_back(3, PESO_CCV);
        if (i + 1 < n && !v(i) && v(i+1)) r.emplace_back(2, PESO_CV);
        if (i + 1 < n && v(i) && !v(i+1)) r.emplace_back(2, PESO_VC);
        if (!v(i)) r.emplace_back(1, PESO_C);
        if (v(i)) r.emplace_back(1, PESO_V);
        return r;
    }

    bool calcular() {
        dp[n] = 0;
        for (int idx = static_cast<int>(n) - 1; idx >= 0; --idx) {
            const auto c = candidatos(static_cast<size_t>(idx));
            long long melhor = NEG;
            size_t melhor_l = std::numeric_limits<size_t>::max();
            for (const auto &par : c) {
                const size_t l = par.first;
                const long long peso = par.second;
                if (static_cast<size_t>(idx) + l <= n && dp[static_cast<size_t>(idx) + l] != NEG) {
                    const long long tot = dp[static_cast<size_t>(idx) + l] + peso;
                    if (tot > melhor) {
                        melhor = tot;
                        melhor_l = l;
                    }
                } else if (static_cast<size_t>(idx) + l == n) {
                    if (peso > melhor) {
                        melhor = peso;
                        melhor_l = l;
                    }
                }
            }
            dp[static_cast<size_t>(idx)] = melhor;
            prox[static_cast<size_t>(idx)] = melhor_l;
        }
        return dp[0] != NEG;
    }

    [[nodiscard]] std::vector<NoAST> reconstruir_ast() const {
        std::vector<NoAST> r;
        NoAST raiz = NoAST::faz_palavra();
        r.push_back(std::move(raiz));
        size_t i = 0;
        while (i < n) {
            const size_t l = prox[i];
            if (l == std::numeric_limits<size_t>::max() || l == 0) {
                std::string s(1, tokens[i].caractere);
                r.push_back(NoAST::faz_silaba(s, tokens[i].pos));
                ++i;
            } else {
                std::string s;
                s.reserve(l);
                const size_t pos_inicio = tokens[i].pos;
                for (size_t k = 0; k < l; ++k) s.push_back(tokens[i + k].caractere);
                r.push_back(NoAST::faz_silaba(s, pos_inicio));
                i += l;
            }
        }
        return r;
    }
};

[[nodiscard]] static std::string tipo_unidade(const std::string &s) {
    const size_t m = s.size();
    if (m == 4 && eh_vogal_char(s[0]) && !eh_vogal_char(s[1]) && !eh_vogal_char(s[2]) && eh_vogal_char(s[3])) return "VCCV";
    if (m == 3 && eh_vogal_char(s[0]) && eh_vogal_char(s[1]) && eh_vogal_char(s[2])) return "VVV";
    if (m == 3 && !eh_vogal_char(s[0]) && !eh_vogal_char(s[1]) && eh_vogal_char(s[2])) return "CCV";
    if (m == 2 && !eh_vogal_char(s[0]) && eh_vogal_char(s[1])) return "CV";
    if (m == 2 && eh_vogal_char(s[0]) && !eh_vogal_char(s[1])) return "VC";
    if (m == 1 && !eh_vogal_char(s[0])) return "C";
    if (m == 1 && eh_vogal_char(s[0])) return "V";
    return "?";
}

static void mostrar_intro() {
    std::cout << "=====================================================" << '\n';
    std::cout << "            ANALISE DE CADEIAS - COMO USAR" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Digite uma palavra e o sistema verificara' se ela e' va'lida" << '\n';
    std::cout << "segundo as regras da linguagem ficticia e mostrara' a divisao" << '\n';
    std::cout << "em unidades (\"silaba\")." << '\n' << '\n';
    std::cout << "Regras principais:" << '\n';
    std::cout << "  1) Apenas letras A-Z ou a-z" << '\n';
    std::cout << "  2) Maximo 3 vogais consecutivas" << '\n';
    std::cout << "  3) Termina em vogal" << '\n';
    std::cout << "  4) Sequencias de consoantes seguem restricoes (ver exemplos)" << '\n';
    std::cout << "Comandos especiais:" << '\n';
    std::cout << "  0 -> sair" << '\n';
    std::cout << "  1 -> executar analisador semantico sobre a ultima palavra valida" << '\n';
    std::cout << "=====================================================" << '\n' << '\n';
}

static void mostrar_semantic_rules() {
    std::cout << "=====================================================" << '\n';
    std::cout << "            ANALISE SEMANTICA - REGRAS" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Cada caractere e' representado por seu valor ASCII:" << '\n';
    std::cout << "  vogal -> positivo (+n)" << '\n';
    std::cout << "  consoante -> negativo (-n)" << '\n' << '\n';
    std::cout << "Conversao por unidade sintatica:" << '\n';
    std::cout << "  V    -> +n" << '\n';
    std::cout << "  C    -> -n" << '\n';
    std::cout << "  VC   -> +n -m" << '\n';
    std::cout << "  CV   -> -m +n" << '\n';
    std::cout << "  CCV  -> *( -c -c +v )  (soma dos valores por posicao)" << '\n';
    std::cout << "  VVV  -> *( +v +v +v )  (soma das tres vogais)" << '\n';
    std::cout << "  VCCV -> ( +v -c ) * ( -c +v )" << '\n';
    std::cout << "A operacao final e' montada lendo as unidades em ordem esquerda->direita." << '\n';
    std::cout << "Exemplo: 'agua' -> a gu a -> x = +97 -103 +117 +97" << '\n';
    std::cout << "=====================================================" << '\n' << '\n';
}

static std::string signed_num(int v) {
    std::ostringstream ss;
    if (v >= 0) ss << "+" << v;
    else ss << v;
    return ss.str();
}

static std::string semantica_para_silaba(const std::string &s) {
    const std::string tipo = tipo_unidade(s);
    if (tipo == "V") return signed_num(static_cast<int>(static_cast<unsigned char>(s[0])));
    if (tipo == "C") return signed_num(-static_cast<int>(static_cast<unsigned char>(s[0])));
    if (tipo == "VC") {
        int v = static_cast<int>(static_cast<unsigned char>(s[0]));
        int c = static_cast<int>(static_cast<unsigned char>(s[1]));
        std::ostringstream ss;
        ss << signed_num(v) << " " << signed_num(-c);
        return ss.str();
    }
    if (tipo == "CV") {
        int c = static_cast<int>(static_cast<unsigned char>(s[0]));
        int v = static_cast<int>(static_cast<unsigned char>(s[1]));
        std::ostringstream ss;
        ss << signed_num(-c) << " " << signed_num(v);
        return ss.str();
    }
    if (tipo == "CCV") {
        int c1 = static_cast<int>(static_cast<unsigned char>(s[0]));
        int c2 = static_cast<int>(static_cast<unsigned char>(s[1]));
        int v  = static_cast<int>(static_cast<unsigned char>(s[2]));
        std::ostringstream ss;
        ss << "*(" << signed_num(-c1) << " " << signed_num(-c2) << " " << signed_num(v) << ")";
        return ss.str();
    }
    if (tipo == "VVV") {
        int v1 = static_cast<int>(static_cast<unsigned char>(s[0]));
        int v2 = static_cast<int>(static_cast<unsigned char>(s[1]));
        int v3 = static_cast<int>(static_cast<unsigned char>(s[2]));
        std::ostringstream ss;
        ss << "*(" << signed_num(v1) << " " << signed_num(v2) << " " << signed_num(v3) << ")";
        return ss.str();
    }
    if (tipo == "VCCV") {
        int v1 = static_cast<int>(static_cast<unsigned char>(s[0]));
        int c1 = static_cast<int>(static_cast<unsigned char>(s[1]));
        int c2 = static_cast<int>(static_cast<unsigned char>(s[2]));
        int v2 = static_cast<int>(static_cast<unsigned char>(s[3]));
        std::ostringstream ss;
        ss << "(" << signed_num(v1) << signed_num(-c1) << ")*(" << signed_num(-c2) << signed_num(v2) << ")";
        return ss.str();
    }
    std::ostringstream ss;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i) ss << " ";
        int val = static_cast<int>(static_cast<unsigned char>(s[i]));
        ss << (eh_vogal_char(s[i]) ? "+" : "-") << val;
    }
    return ss.str();
}

static void executar_semantica(const std::string &palavra) {
    if (palavra.empty()) {
        cout << "Nenhuma palavra salva para analise semantica" << endl;
        return;
    }
    bool ok;
    string erro;
    const auto tokens = tokenizar(palavra, ok, erro);
    if (!ok) {
        cout << "Erro lexico na palavra salva: " << erro << endl;
        return;
    }
    string motivo;
    size_t pos_erro = 0;
    if (!validar_cadeia_globais(palavra, motivo, pos_erro)) {
        cout << "Erro sintatico na palavra salva: " << motivo << " na posicao " << pos_erro << endl;
        return;
    }
    Parser p(tokens);
    if (!p.calcular()) {
        cout << "Erro sintatico: cadeia nao pode ser particionada" << endl;
        return;
    }
    const auto ast = p.reconstruir_ast();

    cout << "Tokens lexicos:" << endl;
    for (const auto &t : tokens) cout << t.caractere << ":" << (t.vogal ? "V" : "C") << "(" << t.pos << ") ";
    cout << endl;
    cout << "Unidades sintaticas:" << endl;
    for (size_t i = 1; i < ast.size(); ++i) {
        const NoAST &n = ast[i];
        const string &s = n.texto;
        const string tipo = tipo_unidade(s);
        const char *tipo_ast = (n.tipo == NoAST::Tipo::Silaba) ? "Silaba" : "Palavra";
        cout << tipo_ast << ":" << tipo << "(" << s << ")@" << n.pos_inicio << " ";
    }
    cout << endl;
    ostringstream expr;
    expr << "x = ";
    bool primeiro = true;
    for (size_t i = 1; i < ast.size(); ++i) {
        const NoAST &n = ast[i];
        const string frag = semantica_para_silaba(n.texto);
        if (!primeiro) expr << " ";
        expr << frag;
        primeiro = false;
    }
    cout << expr.str() << endl;
}

static void processar_palavra(const string &entrada, string &ultima_palavra) {
    bool ok;
    string erro;
    const auto tokens = tokenizar(entrada, ok, erro);
    if (!ok) {
        cout << "Erro lexico: " << erro << endl;
        return;
    }
    string motivo;
    size_t pos_erro = 0;
    if (!validar_cadeia_globais(entrada, motivo, pos_erro)) {
        cout << "Erro sintatico: " << motivo << " na posicao " << pos_erro << endl;
        return;
    }
    Parser p(tokens);
    if (!p.calcular()) {
        cout << "Erro sintatico: cadeia nao pode ser particionada" << endl;
        return;
    }
    const auto ast = p.reconstruir_ast();
    cout << "Tokens lexicos:" << endl;
    for (const auto &t : tokens) cout << t.caractere << ":" << (t.vogal ? "V" : "C") << "(" << t.pos << ") ";
    cout << endl;
    cout << "Unidades sintaticas:" << endl;
    for (size_t i = 1; i < ast.size(); ++i) {
        const NoAST &n = ast[i];
        const string &s = n.texto;
        const string tipo = tipo_unidade(s);
        const char *tipo_ast = (n.tipo == NoAST::Tipo::Silaba) ? "Silaba" : "Palavra";
        cout << tipo_ast << ":" << tipo << "(" << s << ")@" << n.pos_inicio << " ";
    }
    cout << endl;
    cout << "Valida" << endl;
    ultima_palavra = entrada;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    mostrar_intro();
    string ultima_palavra;
    while (true) {
        cout << "> ";
        string linha;
        if (!getline(cin, linha)) break;
        const string entrada = trim(linha);
        if (entrada.empty()) continue;
        if (entrada == "0") break;
        if (entrada == "1") {
            mostrar_semantic_rules();
            executar_semantica(ultima_palavra);
            continue;
        }
        string lower;
        lower.reserve(entrada.size());
        for (char c : entrada) lower.push_back(static_cast<char>(tolower(static_cast<unsigned char>(c))));
        processar_palavra(lower, ultima_palavra);
    }
    return 0;
}