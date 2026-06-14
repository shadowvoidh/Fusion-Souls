#pragma once
// =============================================================================
//  Combat.hpp
//  Sistema de combate: cálculo de dano, projéteis, colisões e resolução.
// =============================================================================

#include <vector>
#include <cmath>
#include <SFML/System/Vector2.hpp>
#include "Entity.hpp"
#include "CardSystem.hpp"

// ---------------------------------------------------------------------------
//  Projétil
// ---------------------------------------------------------------------------
struct Projetil {
    sf::Vector2f pos;
    sf::Vector2f vel;          // pixels/s
    float        raio      = 5.f;
    float        dano      = 0.f;
    bool         magico    = false;
    bool         fogo      = false;
    bool         veneno    = false;
    float        duracaoVeneno = 0.f;
    float        potVeneno = 0.f;
    float        vida      = 3.0f;   // segundos até expirar
    bool         ativo     = true;
    int          teamID    = 0;      // 0 = jogador, 1 = inimigo
    sf::Color    cor       = sf::Color::White;
    bool         perfurante = false; // atravessa inimigos (SniperVision)
    float        slowPct   = 0.f;    // Chronoguard / Slowex
    float        duracaoSlow = 0.f;
    TipoStatus   statusAplicado = TipoStatus::Nenhum;
    float        duracaoStatus  = 0.f;
    float        potenciaStatus = 0.f;
};

// ---------------------------------------------------------------------------
//  Área de Efeito (explosão, nuvem, etc.)
// ---------------------------------------------------------------------------
struct AreaEfeito {
    sf::Vector2f pos;
    float        raio      = 50.f;
    float        dano      = 0.f;
    float        duracao   = 0.f;   // 0 = instantâneo
    float        intervalo = 0.5f;  // dano por tick
    float        timer     = 0.f;
    bool         magico    = false;
    bool         ativo     = true;
    int          teamID    = 0;
    sf::Color    cor       = sf::Color(255, 100, 0, 80);
    TipoStatus   statusAplicado = TipoStatus::Nenhum;
    float        duracaoStatus  = 0.f;
    float        potenciaStatus = 0.f;
    float        slowPct   = 0.f;   // Toxishaman slow no campo da nuvem
};

// ---------------------------------------------------------------------------
//  Resultado de um ataque para feedback visual / audio
// ---------------------------------------------------------------------------
struct ResultadoAtaque {
    sf::Vector2f posImpacto;
    float        dano    = 0.f;
    bool         critico = false;
    bool         bloqueado = false;
    bool         mortal  = false;
};

// ---------------------------------------------------------------------------
//  Motor de Combate
// ---------------------------------------------------------------------------
class MotorCombate {
public:
    std::vector<Projetil>   projeteis;
    std::vector<AreaEfeito> areas;
    std::vector<ResultadoAtaque> resultados;  // Para partículas / HUD

    // ── Criação de projétil ────────────────────────────────────────────
    void dispararProjetil(
        sf::Vector2f origem, sf::Vector2f direcao, float velocidade,
        float dano, bool magico, int teamID, sf::Color cor,
        bool fogo = false, bool veneno = false,
        float duracaoVeneno = 0.f, float potVeneno = 0.f,
        bool perfurante = false, float slowPct = 0.f, float duracaoSlow = 0.f,
        TipoStatus status = TipoStatus::Nenhum,
        float duracaoStatus = 0.f, float potStatus = 0.f)
    {
        float len = std::hypot(direcao.x, direcao.y);
        if (len < 0.001f) return;
        direcao /= len;

        Projetil p;
        p.pos     = origem;
        p.vel     = direcao * velocidade;
        p.dano    = dano;
        p.magico  = magico;
        p.fogo    = fogo;
        p.veneno  = veneno;
        p.duracaoVeneno = duracaoVeneno;
        p.potVeneno = potVeneno;
        p.teamID  = teamID;
        p.cor     = cor;
        p.perfurante = perfurante;
        p.slowPct = slowPct;
        p.duracaoSlow = duracaoSlow;
        p.statusAplicado = status;
        p.duracaoStatus  = duracaoStatus;
        p.potenciaStatus = potStatus;
        projeteis.push_back(p);
    }

    // Dispara múltiplos projéteis em leque (Saraivada Fantasma)
    void dispararLeque(
        sf::Vector2f origem, sf::Vector2f direcaoCentral, float velocidade,
        float dano, int numProjeteis, float anguloTotal, bool magico,
        int teamID, sf::Color cor)
    {
        float anguloBase = std::atan2(direcaoCentral.y, direcaoCentral.x);
        float step = numProjeteis > 1 ? anguloTotal / (numProjeteis - 1) : 0.f;
        float inicio = anguloBase - anguloTotal / 2.f;
        for (int i = 0; i < numProjeteis; ++i) {
            float ang = inicio + step * i;
            sf::Vector2f dir = {std::cos(ang), std::sin(ang)};
            dispararProjetil(origem, dir, velocidade, dano, magico, teamID, cor);
        }
    }

    // ── Criação de AoE ────────────────────────────────────────────────
    void criarArea(sf::Vector2f pos, float raio, float dano,
                   float duracao, bool magico, int teamID, sf::Color cor,
                   TipoStatus status = TipoStatus::Nenhum,
                   float duracaoStatus = 0.f, float potStatus = 0.f,
                   float slowPct = 0.f)
    {
        AreaEfeito a;
        a.pos     = pos;
        a.raio    = raio;
        a.dano    = dano;
        a.duracao = duracao;
        a.magico  = magico;
        a.teamID  = teamID;
        a.cor     = cor;
        a.statusAplicado = status;
        a.duracaoStatus  = duracaoStatus;
        a.potenciaStatus = potStatus;
        a.slowPct = slowPct;
        areas.push_back(a);
    }

    // ── Cálculo de dano crítico ───────────────────────────────────────
    float aplicarCritico(float dano, const Stats& stats, bool forcaCritico = false) {
        float roll = (float)rand() / RAND_MAX;
        if (forcaCritico || roll < stats.critChance) {
            return dano * stats.critMult;
        }
        return dano;
    }

    // ── Ataque melee direto ───────────────────────────────────────────
    ResultadoAtaque ataqueCorpoACorpo(
        EntidadeBase& atacante, EntidadeBase& alvo,
        float multiplicadorDano = 1.0f, bool forcaCritico = false)
    {
        ResultadoAtaque res;
        res.posImpacto = alvo.pos;

        float danoBase = atacante.stats.atk * multiplicadorDano;
        float danoFinal = aplicarCritico(danoBase, atacante.stats, forcaCritico);
        res.critico = (danoFinal > danoBase);

        float danoReal = alvo.receberDano(danoFinal, false);
        res.dano    = danoReal;
        res.mortal  = !alvo.vivo;

        resultados.push_back(res);
        return res;
    }

    // ── Update: move projéteis, verifica colisões ─────────────────────
    void update(float dt, Jogador& jogador, std::vector<Inimigo>& inimigos) {
        resultados.clear();

        // ── Projéteis ──────────────────────────────────────────────────
        for (auto& p : projeteis) {
            if (!p.ativo) continue;
            p.pos  += p.vel * dt;
            p.vida -= dt;
            if (p.vida <= 0.f) { p.ativo = false; continue; }

            // Projéteis do jogador → acerta inimigos
            if (p.teamID == 0) {
                for (auto& e : inimigos) {
                    if (!e.vivo) continue;
                    float dist = std::hypot(e.pos.x - p.pos.x, e.pos.y - p.pos.y);
                    if (dist <= p.raio + 18.f) {  // raio do inimigo ~18px
                        ResultadoAtaque res;
                        res.posImpacto = p.pos;
                        res.dano = e.receberDano(p.dano, p.magico, p.fogo);
                        res.mortal = !e.vivo;

                        if (p.veneno)
                            e.aplicarStatus(TipoStatus::Veneno, p.duracaoVeneno, p.potVeneno);
                        if (p.slowPct > 0.f)
                            e.aplicarStatus(TipoStatus::Lento, p.duracaoSlow, p.slowPct);
                        if (p.statusAplicado != TipoStatus::Nenhum)
                            e.aplicarStatus(p.statusAplicado, p.duracaoStatus, p.potenciaStatus);

                        resultados.push_back(res);
                        if (!p.perfurante) { p.ativo = false; break; }
                    }
                }
            }
            // Projéteis dos inimigos → acerta jogador
            else {
                if (!jogador.invulneravel && jogador.iframeTimer <= 0.f) {
                    float dist = std::hypot(jogador.pos.x - p.pos.x, jogador.pos.y - p.pos.y);
                    if (dist <= p.raio + 16.f) {
                        ResultadoAtaque res;
                        res.posImpacto = p.pos;
                        res.dano = jogador.receberDano(p.dano, p.magico, p.fogo);
                        res.mortal = !jogador.vivo;

                        if (p.veneno)
                            jogador.aplicarStatus(TipoStatus::Veneno, p.duracaoVeneno, p.potVeneno);
                        if (p.slowPct > 0.f)
                            jogador.aplicarStatus(TipoStatus::Lento, p.duracaoSlow, p.slowPct);

                        resultados.push_back(res);
                        p.ativo = false;
                    }
                }
            }
        }

        // Remove projéteis inativos
        projeteis.erase(
            std::remove_if(projeteis.begin(), projeteis.end(),
                [](const Projetil& p) { return !p.ativo; }),
            projeteis.end());

        // ── Áreas de Efeito ────────────────────────────────────────────
        for (auto& a : areas) {
            if (!a.ativo) continue;

            if (a.duracao > 0.f) {
                a.duracao -= dt;
                a.timer   -= dt;
                if (a.duracao <= 0.f) { a.ativo = false; continue; }
                if (a.timer > 0.f) continue;
                a.timer = a.intervalo;
            }

            auto verificarHit = [&](EntidadeBase& alvo) {
                float dist = std::hypot(alvo.pos.x - a.pos.x, alvo.pos.y - a.pos.y);
                if (dist > a.raio) return;

                if (a.dano > 0.f) alvo.receberDano(a.dano, a.magico);
                if (a.statusAplicado != TipoStatus::Nenhum)
                    alvo.aplicarStatus(a.statusAplicado, a.duracaoStatus, a.potenciaStatus);
                if (a.slowPct > 0.f)
                    alvo.aplicarStatus(TipoStatus::Lento, a.intervalo + 0.1f, a.slowPct);
            };

            if (a.teamID == 0) {
                for (auto& e : inimigos) if (e.vivo) verificarHit(e);
            } else {
                verificarHit(jogador);
            }

            if (a.duracao <= 0.f) a.ativo = false;  // instantânea
        }

        areas.erase(
            std::remove_if(areas.begin(), areas.end(),
                [](const AreaEfeito& a) { return !a.ativo; }),
            areas.end());

        // ── Status effects tick ────────────────────────────────────────
        jogador.updateStatus(dt);
        for (auto& e : inimigos) e.updateStatus(dt);

        // ── Regeneração passiva de mana/stamina ────────────────────────
        jogador.manaAtual    = std::min(jogador.stats.mana,
                                   jogador.manaAtual    + 8.f  * dt);
        jogador.staminaAtual = std::min(jogador.stats.stamina,
                                   jogador.staminaAtual + 15.f * dt);

        // ── Chronoguard: snapshot periódico ───────────────────────────
        static float snapshotTimer = 0.f;
        snapshotTimer -= dt;
        if (snapshotTimer <= 0.f) {
            snapshotTimer = 0.1f;  // 10 snapshots/s
            jogador.registrarSnapshot();
        }

        // Berzerker: bônus de fúria decai lentamente fora de combate
        if (jogador.cargasFuria > 0) {
            static float furyDecayTimer = 0.f;
            furyDecayTimer -= dt;
            if (furyDecayTimer <= 0.f) {
                furyDecayTimer = 3.0f;
                jogador.cargasFuria = std::max(0, jogador.cargasFuria - 1);
                jogador.bonusFuria  = jogador.cargasFuria * 0.05f;
            }
        }
    }

    // ── Executa uma carta do jogador ──────────────────────────────────
    bool executarCarta(Carta& carta, Jogador& jogador,
                       sf::Vector2f direcaoMira,
                       std::vector<Inimigo>& inimigos)
    {
        if (!jogador.deck.usarCarta(carta, jogador.manaAtual, jogador.staminaAtual))
            return false;

        float danoTotal = carta.potencia *
            (carta.efeito == EfeitoCarta::DanoFisico ? jogador.stats.atk :
             carta.efeito == EfeitoCarta::DanoMagico  ? jogador.stats.atkMagico :
             carta.efeito == EfeitoCarta::DanoVeneno  ? jogador.stats.poisonDmg :
             carta.potencia);

        // Bonus Berzerker
        if (carta.linhagem == Linhagem::Berzerker)
            danoTotal *= (1.f + jogador.bonusFuria);

        switch(carta.efeito) {
            case EfeitoCarta::DanoFisico:
            case EfeitoCarta::Knockback:
            {
                // Melee: acerta inimigo mais próximo na direção
                Inimigo* melhorAlvo = nullptr;
                float melhorDist = carta.alcance * 32.f;  // em pixels
                for (auto& e : inimigos) {
                    if (!e.vivo) continue;
                    float dist = std::hypot(e.pos.x - jogador.pos.x,
                                            e.pos.y - jogador.pos.y);
                    if (dist < melhorDist) {
                        melhorDist = dist;
                        melhorAlvo = &e;
                    }
                }
                if (melhorAlvo) {
                    auto res = ataqueCorpoACorpo(jogador, *melhorAlvo, carta.potencia);
                    // Knockback
                    if (carta.efeito == EfeitoCarta::Knockback) {
                        sf::Vector2f dir = melhorAlvo->pos - jogador.pos;
                        float len = std::hypot(dir.x, dir.y);
                        if (len > 0.f) melhorAlvo->pos += dir / len * 60.f;
                        melhorAlvo->aplicarStatus(TipoStatus::Atordoado, carta.duracao, 0.f);
                    }
                    // Acumula fúria se for Berzerker
                    if (carta.linhagem == Linhagem::Berzerker) {
                        jogador.cargasFuria = std::min(jogador.maxCargasFuria,
                                                       jogador.cargasFuria + 1);
                        jogador.bonusFuria = jogador.cargasFuria * 0.05f;
                    }
                }
                break;
            }

            case EfeitoCarta::Projetil:
            {
                bool  fogo   = (carta.linhagem == Linhagem::Dracconico);
                bool  veneno = (carta.linhagem == Linhagem::Toxishaman);
                bool  perf   = (carta.linhagem == Linhagem::SniperVision);
                float vel    = fogo ? 400.f : perf ? 600.f : 350.f;
                sf::Color cor = fogo ? sf::Color(255, 120, 0) :
                                veneno ? sf::Color(80, 220, 60) :
                                perf   ? sf::Color(200, 230, 255) :
                                sf::Color::White;
                float dano = danoTotal;
                dano = aplicarCritico(dano, jogador.stats);

                // Saraivada Fantasma: 5 projéteis em leque
                if (carta.nome == "Saraivada Fantasma") {
                    dispararLeque(jogador.pos, direcaoMira, vel, dano * 0.6f,
                                  5, 0.8f, false, 0, cor);
                } else {
                    dispararProjetil(jogador.pos, direcaoMira, vel, dano,
                        (carta.linhagem == Linhagem::Dracconico ||
                         carta.linhagem == Linhagem::Zexcromante),
                        0, cor,
                        fogo, veneno,
                        veneno ? carta.duracao : 0.f,
                        veneno ? jogador.stats.poisonDmg : 0.f,
                        perf);
                }
                break;
            }

            case EfeitoCarta::AoE:
            {
                sf::Color cor =
                    carta.linhagem == Linhagem::Dracconico ? sf::Color(255, 100, 0, 120) :
                    carta.linhagem == Linhagem::Aegisknight ? sf::Color(100, 150, 255, 100) :
                    carta.linhagem == Linhagem::Toxishaman  ? sf::Color(60, 220, 30, 100) :
                    sf::Color(200, 200, 255, 100);

                float duracaoArea = carta.duracao > 0.f ? carta.duracao : 0.f;
                TipoStatus status =
                    carta.linhagem == Linhagem::Toxishaman ? TipoStatus::Veneno :
                    carta.linhagem == Linhagem::Dracconico ? TipoStatus::Queimadura :
                    carta.linhagem == Linhagem::Aegisknight? TipoStatus::Atordoado :
                    TipoStatus::Nenhum;

                criarArea(jogador.pos, carta.alcance * 32.f, danoTotal,
                          duracaoArea, true, 0, cor,
                          status, carta.duracao, 8.f,
                          carta.linhagem == Linhagem::Toxishaman ? 0.3f : 0.f);
                break;
            }

            case EfeitoCarta::Cura:
            {
                float cura = jogador.stats.hp * carta.potencia;
                jogador.curar(cura);
                break;
            }

            case EfeitoCarta::Escudo:
            {
                jogador.shieldAtual = std::min(
                    jogador.stats.shieldHP,
                    jogador.shieldAtual + carta.potencia);
                break;
            }

            case EfeitoCarta::DashEvasao:
            {
                // Ativa dash com iframes
                float len = std::hypot(direcaoMira.x, direcaoMira.y);
                if (len > 0.f) {
                    jogador.dashDir   = direcaoMira / len;
                    jogador.dashTimer = 0.25f;
                    jogador.dashAtivo = true;
                    jogador.iframeTimer = carta.duracao > 0.f ? carta.duracao : 0.4f;

                    // Toxishaman: deixa nuvem no ponto de origem
                    if (jogador.fusao.get(Linhagem::Toxishaman) >= 0.25f) {
                        criarArea(jogador.pos, 60.f, jogador.stats.poisonDmg * 0.3f,
                                  3.0f, true, 0, sf::Color(60, 220, 30, 80),
                                  TipoStatus::Veneno, 3.0f, 5.f, 0.2f);
                    }
                }
                break;
            }

            case EfeitoCarta::SlowTempo:
            {
                float slow = jogador.stats.timeSlowPct;
                for (auto& e : inimigos) {
                    float dist = std::hypot(e.pos.x - jogador.pos.x,
                                            e.pos.y - jogador.pos.y);
                    if (dist <= carta.alcance * 32.f)
                        e.aplicarStatus(TipoStatus::Lento, carta.duracao, slow);
                }
                break;
            }

            case EfeitoCarta::InvocarEspirito:
                // A lógica de espíritos é gerenciada pelo Game (entidades separadas)
                // Aqui só sinalizamos ao motor
                jogador.numEspiritos = std::min(
                    (int)jogador.stats.spiritCount,
                    jogador.numEspiritos + 1);
                break;

            case EfeitoCarta::RebobinarVida:
                jogador.rebobinar();
                break;

            case EfeitoCarta::CargaFrenesi:
                jogador.cargasFuria = std::min(jogador.maxCargasFuria,
                                               jogador.cargasFuria + 1);
                jogador.bonusFuria  = jogador.cargasFuria * 0.05f;
                break;

            case EfeitoCarta::PassivaToggle:
                // Mira Crítica: próximo ataque é crítico garantido
                // Implementado via flag no jogador (simplificado)
                break;

            default: break;
        }
        return true;
    }

    void limpar() {
        projeteis.clear();
        areas.clear();
        resultados.clear();
    }
};