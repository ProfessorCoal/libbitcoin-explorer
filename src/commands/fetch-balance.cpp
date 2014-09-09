/**
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin_explorer.
 *
 * libbitcoin_explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "precompile.hpp"
#include <explorer/commands/fetch-balance.hpp>

#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <explorer/callback_state.hpp>
#include <explorer/define.hpp>
#include <explorer/obelisk_client.hpp>
#include <explorer/prop_tree.hpp>

using namespace bc;
using namespace explorer;
using namespace explorer::commands;
using namespace explorer::primitives;

static void handle_callback(callback_state& state,
    const payment_address& address, const std::vector<balance_row>& histories)
{
    const auto tree = prop_tree(histories, address);
    state.output(tree);
    --state;
}

console_result fetch_balance::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& encoding = get_format_option();
    const auto& addresses = get_bitcoin_addresss_argument();

    callback_state state(error, output, encoding);
    const auto handler = [&state](const payment_address& address,
        const std::error_code& code, const std::vector<balance_row>& history)
    {
        if (!state.handle_error(code))
            handle_callback(state, address, history);
    };

    obelisk_client client(*this);
    auto& fullnode = client.get_fullnode();
    for (const auto& address: addresses)
    {
        ++state;
        fullnode.address.fetch_history(address, 
            std::bind(handler, address, ph::_1, ph::_2));
    }

    client.poll(state.stopped());

    return state.get_result();
}