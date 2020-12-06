#ifndef UNORTHODOX_DFA_TABLE_HPP
#define UNORTHODOX_DFA_TABLE_HPP

#include <vector>
#include <unordered_map>
#include <iostream>
#include <span>
#include <cstring>

// this is a define because I don't really want a type here
#define UDX_ZERO_FLAGS 0

namespace unorthodox {
    template <typename State, typename Transition>
    struct state_transition
    {
        State               current;
        Transition          transition;
        uint8_t             flags = UDX_ZERO_FLAGS;

        friend auto operator<=>(const state_transition&, const state_transition&) = default;
    };

    template <typename S, typename T>
    std::ostream& operator<<(std::ostream& stream, const state_transition<S,T>& t) {
        stream << "[" << t.current << ", " << static_cast<uint32_t>(t.transition) << "]";
        return stream;
    }

    template <typename K, typename V>
    struct dfa_table
    {
        public:
            using key_type                  = K;
            using value_type                = V;
            using size_type                 = std::size_t;
            using difference_type           = std::ptrdiff_t;
            using reference                 = V&;
            using const_reference           = const V&;
            using pointer                   = V*;
            using const_pointer             = const V*;

            using state_type                = int;
            using pack_type                 = std::vector<K>;
            using transition_type           = state_transition<state_type, key_type>;

            template <bool is_iterator_const>
            class iterator_type;

            using iterator                  = iterator_type<false>;
            using const_iterator            = iterator_type<true>;

            using reverse_iterator          = std::reverse_iterator<iterator>;
            using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

            constexpr static state_type INVALID_STATE = -1;
            constexpr static state_type INITIAL_STATE = 0;

            void insert(pack_type, value_type);
            void insert(const char*, value_type);

            state_type transition_result(state_type, key_type) const noexcept;

            value_type entry(pack_type&&);
            value_type entry(const char*);

            void print() const;

        private:
            template <typename MK, typename MV>
            using map_type = std::unordered_map<MK, MV>;

            using transition_map = map_type<transition_type, state_type>;
            using value_map = map_type<state_type, value_type>;

            state_type create_transitions(state_type, std::span<key_type>);

            transition_map  transitions;
            value_map       final_states;
    };

    template <typename K, typename V>
    typename dfa_table<K,V>::state_type dfa_table<K,V>::transition_result(state_type from, key_type where) const noexcept
    {
        auto transition = transition_type {
            .current = from,
            .transition = where
        };

        if (!transitions.contains(transition))
            return INVALID_STATE;

        return transitions[transition];
    }

    template <typename K, typename V>
    typename dfa_table<K,V>::state_type dfa_table<K,V>::create_transitions(
            state_type current_state,
            std::span<key_type> new_transitions)
    {
        auto transition = transition_type {
            .current = current_state,
            .transition = new_transitions.front()
        };

        if (!transitions.contains(transition)) {
            // need to create a new state
            state_type new_state_index = transitions.size() + 1;
            transitions[transition] = (new_state_index);
        }

        if (new_transitions.size() > 1)
            return create_transitions(transitions[transition], {new_transitions.begin() + 1, new_transitions.size() -1});

        return transitions[transition];
    }

    template <typename K, typename V>
    void dfa_table<K,V>::insert(pack_type values, value_type result)
    {
        state_type final_state = create_transitions(INITIAL_STATE, { values.begin(), values.size() });
        final_states[final_state] = result;
    }

    template <typename K, typename V>
    void dfa_table<K,V>::insert(const char* values, value_type result)
    {
        const char8_t* nptr = reinterpret_cast<const char8_t*>(values);
        auto s = std::vector<char8_t>(nptr, nptr + strlen(values));
        state_type final_state = create_transitions(INITIAL_STATE, { s.begin(), s.size() });
        final_states[final_state] = result;
    }

    template <typename K, typename V>
    typename dfa_table<K,V>::value_type dfa_table<K,V>::entry(pack_type&& values)
    {
        state_type current_state = INITIAL_STATE;
        for (auto&& v : values) {
            auto transition = transition_type {
                .current = current_state,
                .transition = v
            };
            current_state = transitions[transition];
        };

        std::cout << current_state << "\n";

        return final_states[current_state];
    }
    
    template <typename K, typename V>
    typename dfa_table<K,V>::value_type dfa_table<K,V>::entry(const char* values)
    {
        static_assert(sizeof(key_type) == sizeof(char));
        const key_type* nptr = reinterpret_cast<const key_type*>(values);
        auto s = std::vector<char8_t>(nptr, nptr + strlen(values));
        return entry(std::move(s));
    }
    
    template <typename K, typename V>
    void dfa_table<K,V>::print() const
    {
        std::cout << "transition table: \n";
        for (auto& entry : transitions)
            std::cout << "transition: " << entry.first << " --> " << entry.second << "\n";

        std::cout << "final state table: \n";
        for (auto& entry : final_states)
            std::cout << "state: " << entry.first << " --> " << entry.second << "\n";
    }
}

namespace std {
    template <typename S, typename T>
    struct hash<unorthodox::state_transition<S,T>>
    {
        size_t operator()(const unorthodox::state_transition<S,T>& tr) const {
            constexpr static size_t prime = 2027;
            size_t hash_value =
                hash<S>{}(tr.current);

            hash_value *= prime;
            hash_value += hash<T>{}(tr.transition);

            return hash_value;
        }
    };
}

#endif
