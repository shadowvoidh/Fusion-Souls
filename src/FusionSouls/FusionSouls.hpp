#pragma once
// =============================================================================
//  FusionSouls.hpp
//  Sistema central de fusão de linhagens e cálculo de atributos derivados.
//  Cada linhagem contribui com pesos diferentes para cada stat.
// =============================================================================

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

// ---------------------------------------------------------------------------
//  Linhagens disponíveis (índices usados em arrays)
// ---------------------------------------------------------------------------
enum class Linhagem {
    Humano      = 0,
    Dracconico  = 1,
    Zexcromante = 2,
    Berzerker   = 3,
    SniperVision= 4,
    Chronoguard = 5,
    Toxishaman  = 6,
    Aegisknight = 7,
    COUNT       = 8
};

inline std::string nomeLinhagem(Linhagem l) {
    switch(l) {
        case Linhagem::Humano:       return "Humano";
        case Linhagem::Dracconico:   return "Dracônico";
        case Linhagem::Zexcromante:  return "Zexcromante";
        case Linhagem::Berzerker:    return "Berzerker";
        case Linhagem::SniperVision: return "SniperVision";
        case Linhagem::Chronoguard:  return "Chronoguard";
        case Linhagem::Toxishaman:   return "Toxishaman";
        case Linhagem::Aegisknight:  return "Aegisknight";
        default:                     return "Desconhecido";
    }
}

// ---------------------------------------------------------------------------
//  Atributos derivados calculados pelo sistema de fusão
// ---------------------------------------------------------------------------
struct Stats {
    float hp        = 0.f;   // Vida máxima
    float atk       = 0.f;   // Ataque físico
    float atkMagico = 0.f;   // Ataque mágico / essência
    float def       = 0.f;   // Defesa física
    float defMagica = 0.f;   // Defesa mágica
    float spd       = 0.f;   // Velocidade de movimento
    float mana      = 0.f;   // Mana / energia de habilidades
    float stamina   = 0.f;   // Estamina para ações físicas
    float range     = 0.f;   // Alcance de ataques (1.0 = melee, 10.0 = sniper)
    float critChance= 0.f;   // Chance de crítico (0..1)
    float critMult  = 1.5f;  // Multiplicador de crítico
    float poisonDmg = 0.f;   // Dano por segundo de veneno (Toxishaman)
    float shieldHP  = 0.f;   // HP do escudo (Aegisknight)
    float timeSlowPct= 0.f;  // % de slow do tempo (Chronoguard 0..1)
    float spiritCount= 0.f;  // Número de espíritos invocados (Zexcromante)
    float fireResist= 0.f;   // Resistência a fogo (Dracônico passiva)
};

// ---------------------------------------------------------------------------
//  Tabela de pesos: quanto cada linhagem contribui por 1.0 (100%) de fusão
//  Formato: Stats base * (porcentagem da linhagem)
// ---------------------------------------------------------------------------
struct PesoLinhagem {
    // Pesos por stat (contribuição quando fusão = 100%)
    float hp         = 0.f;
    float atk        = 0.f;
    float atkMagico  = 0.f;
    float def        = 0.f;
    float defMagica  = 0.f;
    float spd        = 0.f;
    float mana       = 0.f;
    float stamina    = 0.f;
    float range      = 0.f;
    float critChance = 0.f;
    float critMult   = 0.f;
    float poisonDmg  = 0.f;
    float shieldHP   = 0.f;
    float timeSlowPct= 0.f;
    float spiritCount= 0.f;
    float fireResist = 0.f;
};

// Retorna os pesos de cada linhagem
inline PesoLinhagem getPeso(Linhagem l) {
    PesoLinhagem p;
    switch(l) {
        case Linhagem::Humano:
            // Generalista: todas as stats com bônus moderados
            p.hp        = 150.f;
            p.atk       = 40.f;
            p.atkMagico = 20.f;
            p.def       = 30.f;
            p.defMagica = 20.f;
            p.spd       = 40.f;
            p.mana      = 50.f;
            p.stamina   = 80.f;
            p.range     = 1.5f;
            p.critChance= 0.08f;
            p.critMult  = 0.2f;
            break;

        case Linhagem::Dracconico:
            // Mobilidade aérea + fogo: SPD, ATK mágico, resistência a fogo
            p.hp        = 120.f;
            p.atk       = 55.f;
            p.atkMagico = 90.f;   // Projéteis de fogo
            p.def       = 20.f;
            p.defMagica = 40.f;
            p.spd       = 85.f;   // Mobilidade aérea
            p.mana      = 70.f;
            p.stamina   = 50.f;
            p.range     = 4.0f;   // Projéteis médio alcance
            p.critChance= 0.12f;
            p.critMult  = 0.4f;
            p.fireResist= 0.60f;  // 60% resistência a fogo em 100%
            break;

        case Linhagem::Zexcromante:
            // Invocação: MANA alto, atk mágico, espíritos
            p.hp        = 100.f;
            p.atk       = 30.f;
            p.atkMagico = 110.f;  // Espíritos guerreiros
            p.def       = 15.f;
            p.defMagica = 60.f;
            p.spd       = 35.f;
            p.mana      = 130.f;  // Custo alto de invocação
            p.stamina   = 40.f;
            p.range     = 6.0f;   // Espíritos atacam à distância
            p.critChance= 0.06f;
            p.critMult  = 0.3f;
            p.spiritCount= 3.0f;  // Até 3 espíritos com 100%
            break;

        case Linhagem::Berzerker:
            // Tanque ofensivo: HP e ATK físico altíssimos
            p.hp        = 220.f;  // Maior HP do jogo
            p.atk       = 100.f;  // Maior ATK físico
            p.atkMagico = 10.f;
            p.def       = 55.f;
            p.defMagica = 15.f;
            p.spd       = 30.f;   // Lento mas imparável
            p.mana      = 30.f;
            p.stamina   = 120.f;  // Máximo de estamina
            p.range     = 1.0f;   // Corpo a corpo
            p.critChance= 0.18f;  // Alta chance de crítico
            p.critMult  = 0.8f;   // Críticos devastadores
            break;

        case Linhagem::SniperVision:
            // Longo alcance: RANGE, crítico, ataque preciso
            p.hp        = 80.f;   // Frágil
            p.atk       = 70.f;
            p.atkMagico = 30.f;
            p.def       = 10.f;
            p.defMagica = 10.f;
            p.spd       = 50.f;
            p.mana      = 60.f;
            p.stamina   = 70.f;
            p.range     = 12.0f;  // Maior alcance
            p.critChance= 0.28f;  // Sniper = crítico garantido
            p.critMult  = 1.2f;   // Dano de crítico extremo
            break;

        case Linhagem::Chronoguard:
            // Tempo: SPD extremo, esquiva com iframes, slow
            p.hp        = 110.f;
            p.atk       = 35.f;
            p.atkMagico = 55.f;
            p.def       = 25.f;
            p.defMagica = 70.f;   // Proteção temporal
            p.spd       = 100.f;  // Mais rápido do jogo
            p.mana      = 90.f;
            p.stamina   = 80.f;
            p.range     = 2.5f;
            p.critChance= 0.10f;
            p.critMult  = 0.3f;
            p.timeSlowPct= 0.40f; // Até 40% de slow em 100%
            break;

        case Linhagem::Toxishaman:
            // Veneno + evasão em esporos: dano contínuo, mobilidade média
            p.hp        = 95.f;
            p.atk       = 45.f;
            p.atkMagico = 60.f;
            p.def       = 20.f;
            p.defMagica = 45.f;
            p.spd       = 55.f;
            p.mana      = 80.f;
            p.stamina   = 65.f;
            p.range     = 3.5f;
            p.critChance= 0.09f;
            p.critMult  = 0.2f;
            p.poisonDmg = 25.f;   // 25 dps de veneno em 100%
            break;

        case Linhagem::Aegisknight:
            // Tank defensivo: DEF máxima, escudo colossal
            p.hp        = 200.f;
            p.atk       = 60.f;   // Choque magnético
            p.atkMagico = 30.f;
            p.def       = 130.f;  // Maior DEF
            p.defMagica = 80.f;
            p.spd       = 20.f;   // O mais lento
            p.mana      = 50.f;
            p.stamina   = 100.f;
            p.range     = 2.0f;
            p.critChance= 0.05f;
            p.critMult  = 0.1f;
            p.shieldHP  = 150.f;  // Escudo absorve 150 dmg em 100%
            break;

        default: break;
    }
    return p;
}

// ---------------------------------------------------------------------------
//  Registro de fusão do jogador
// ---------------------------------------------------------------------------
struct FusionSouls {
    float pct[static_cast<int>(Linhagem::COUNT)] = {};

    // Construtor padrão: 100% Humano
    FusionSouls() { pct[static_cast<int>(Linhagem::Humano)] = 1.0f; }

    // Normaliza para soma = 1.0 (100%)
    void normalizar() {
        float soma = 0.f;
        for (int i = 0; i < static_cast<int>(Linhagem::COUNT); ++i)
            soma += pct[i];
        if (soma > 0.f)
            for (int i = 0; i < static_cast<int>(Linhagem::COUNT); ++i)
                pct[i] /= soma;
    }

    // Define a fusão a partir de pares {Linhagem, porcentagem}
    void aplicar(std::vector<std::pair<Linhagem, float>> mix) {
        for (int i = 0; i < static_cast<int>(Linhagem::COUNT); ++i)
            pct[i] = 0.f;
        for (auto& [l, v] : mix)
            pct[static_cast<int>(l)] = v;
        normalizar();
    }

    float get(Linhagem l) const { return pct[static_cast<int>(l)]; }

    // Retorna a linhagem dominante
    Linhagem dominante() const {
        int best = 0;
        for (int i = 1; i < static_cast<int>(Linhagem::COUNT); ++i)
            if (pct[i] > pct[best]) best = i;
        return static_cast<Linhagem>(best);
    }

    // Retorna as N linhagens com maior porcentagem (para exibição)
    std::vector<std::pair<Linhagem, float>> top(int n = 3) const {
        std::vector<std::pair<Linhagem, float>> v;
        for (int i = 0; i < static_cast<int>(Linhagem::COUNT); ++i)
            if (pct[i] > 0.005f)
                v.push_back({static_cast<Linhagem>(i), pct[i]});
        std::sort(v.begin(), v.end(),
            [](auto& a, auto& b){ return a.second > b.second; });
        if ((int)v.size() > n) v.resize(n);
        return v;
    }
};

// ---------------------------------------------------------------------------
//  Cálculo de Stats derivados
//  Cada stat é a soma ponderada das contribuições de cada linhagem ativa.
//  Base fixa (Lv 1) + bônus de fusão + bônus de nível.
// ---------------------------------------------------------------------------
inline Stats calcularStats(const FusionSouls& f, int nivel = 1) {
    Stats s;

    // Zera e acumula
    for (int i = 0; i < static_cast<int>(Linhagem::COUNT); ++i) {
        float p = f.pct[i];
        if (p < 0.001f) continue;
        const PesoLinhagem w = getPeso(static_cast<Linhagem>(i));

        s.hp         += w.hp          * p;
        s.atk        += w.atk         * p;
        s.atkMagico  += w.atkMagico   * p;
        s.def        += w.def         * p;
        s.defMagica  += w.defMagica   * p;
        s.spd        += w.spd         * p;
        s.mana       += w.mana        * p;
        s.stamina    += w.stamina     * p;
        s.range      += w.range       * p;
        s.critChance += w.critChance  * p;
        s.critMult   += w.critMult    * p;
        s.poisonDmg  += w.poisonDmg   * p;
        s.shieldHP   += w.shieldHP    * p;
        s.timeSlowPct+= w.timeSlowPct * p;
        s.spiritCount+= w.spiritCount * p;
        s.fireResist += w.fireResist  * p;
    }

    // Escala por nível (crescimento linear simples para a base)
    float lvScale = 1.0f + (nivel - 1) * 0.12f;  // +12% por nível
    s.hp         = std::floor(s.hp        * lvScale);
    s.atk        = std::floor(s.atk       * lvScale);
    s.atkMagico  = std::floor(s.atkMagico * lvScale);
    s.def        = std::floor(s.def       * lvScale);
    s.defMagica  = std::floor(s.defMagica * lvScale);
    s.mana       = std::floor(s.mana      * lvScale);
    s.stamina    = std::floor(s.stamina   * lvScale);
    s.shieldHP   = std::floor(s.shieldHP  * lvScale);
    // SPD, range, crít, etc. não escalam com nível (são derivados de mecânica)

    // Clamp de segurança
    s.critChance  = std::min(s.critChance,  0.95f);
    s.timeSlowPct = std::min(s.timeSlowPct, 0.75f);
    s.fireResist  = std::min(s.fireResist,  0.90f);
    s.critMult    = 1.5f + s.critMult;       // Base 1.5x + bônus de fusão
    s.spiritCount = std::floor(s.spiritCount);

    return s;
}

// ---------------------------------------------------------------------------
//  Passivas ativas por threshold de fusão
//  Ex: se dracônico >= 30%, ganha passiva "Escamas de Fogo"
// ---------------------------------------------------------------------------
struct Passiva {
    std::string nome;
    std::string descricao;
    Linhagem    origem;
    float       threshold; // mínimo % para ativar
};

inline std::vector<Passiva> passivas() {
    return {
        {"Escamas de Fogo",    "Resistência a fogo +60%. Dano de fogo +20%.",  Linhagem::Dracconico,   0.20f},
        {"Voo Rasante",        "Velocidade aumenta 30% ao desviar.",            Linhagem::Dracconico,   0.40f},
        {"Espíritos Ligados",  "Espíritos absorvem 10% do dano recebido.",      Linhagem::Zexcromante,  0.25f},
        {"Frenesi de Sangue",  "ATK +15% abaixo de 30% HP.",                   Linhagem::Berzerker,    0.30f},
        {"Pele de Ferro",      "DEF +20% ao ficar parado 2s.",                  Linhagem::Berzerker,    0.50f},
        {"Olho de Águia",      "Critérios revelam posição inimiga 3s.",         Linhagem::SniperVision, 0.20f},
        {"Headshot Temporal",  "Críticos com alcance máximo +50% de dano.",     Linhagem::SniperVision, 0.45f},
        {"Bolso do Tempo",     "Rebobina vida 2s ao receber golpe fatal (1x).", Linhagem::Chronoguard,  0.35f},
        {"Névoa de Esporos",   "Desvio deixa nuvem tóxica por 3s.",             Linhagem::Toxishaman,   0.25f},
        {"Veneno Necrófago",   "Inimigos envenenados curam o jogador 5 HP/s.",  Linhagem::Toxishaman,   0.50f},
        {"Escudo Absoluto",    "Escudo reflete 15% do dano bloqueado.",         Linhagem::Aegisknight,  0.30f},
        {"Onda Magnética",     "Bloqueio perfeito gera knockback em área.",     Linhagem::Aegisknight,  0.55f},
    };
}

inline std::vector<Passiva> passivasAtivas(const FusionSouls& f) {
    std::vector<Passiva> ativas;
    for (const auto& p : passivas())
        if (f.get(p.origem) >= p.threshold)
            ativas.push_back(p);
    return ativas;
}

// ---------------------------------------------------------------------------
//  Nome visual da fusão (para HUD e narrativa)
// ---------------------------------------------------------------------------
inline std::string nomeFusao(const FusionSouls& f) {
    auto top2 = f.top(2);
    if (top2.empty()) return "Vazio";

    auto dom = top2[0].first;
    float pDom = top2[0].second;

    // Pura (>= 85%)
    if (pDom >= 0.85f) {
        switch(dom) {
            case Linhagem::Humano:       return "Humano Puro";
            case Linhagem::Dracconico:   return "Senhor dos Dragões";
            case Linhagem::Zexcromante:  return "Arquimago Espectral";
            case Linhagem::Berzerker:    return "Titã Imortal";
            case Linhagem::SniperVision: return "Olho do Abismo";
            case Linhagem::Chronoguard:  return "Guardião Eterno";
            case Linhagem::Toxishaman:   return "Praga Encarnada";
            case Linhagem::Aegisknight:  return "Muralha Viva";
            default:                     return "Linhagem Pura";
        }
    }

    // Dupla fusão (titulos especiais para combinações narrativas)
    if (top2.size() >= 2) {
        auto sec = top2[1].first;
        // Verifica pares específicos do lore
        auto par = std::make_pair(dom, sec);
        if (par == std::make_pair(Linhagem::Dracconico, Linhagem::Berzerker) ||
            par == std::make_pair(Linhagem::Berzerker, Linhagem::Dracconico))
            return "Dragão Furioso";
        if (par == std::make_pair(Linhagem::Chronoguard, Linhagem::SniperVision) ||
            par == std::make_pair(Linhagem::SniperVision, Linhagem::Chronoguard))
            return "Atirador Fantasma";
        if (par == std::make_pair(Linhagem::Zexcromante, Linhagem::Toxishaman) ||
            par == std::make_pair(Linhagem::Toxishaman, Linhagem::Zexcromante))
            return "Xamã das Almas Podres";
        if (par == std::make_pair(Linhagem::Aegisknight, Linhagem::Berzerker) ||
            par == std::make_pair(Linhagem::Berzerker, Linhagem::Aegisknight))
            return "Colosso de Guerra";
        if (par == std::make_pair(Linhagem::Dracconico, Linhagem::Chronoguard) ||
            par == std::make_pair(Linhagem::Chronoguard, Linhagem::Dracconico))
            return "Dragão do Instante";

        return nomeLinhagem(dom) + "/" + nomeLinhagem(sec);
    }

    return nomeLinhagem(dom);
}