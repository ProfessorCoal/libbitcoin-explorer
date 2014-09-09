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
#include <explorer/commands/send-tx.hpp>

#include <iostream>
#include <boost/format.hpp>
#include <bitcoin/bitcoin.hpp>
#include <explorer/callback_state.hpp>
#include <explorer/define.hpp>
#include <explorer/obelisk_client.hpp>
#include <explorer/utility/utility.hpp>

using namespace bc;
using namespace explorer;
using namespace explorer::commands;

static void handle_callback(callback_state& state)
{
    state.output(boost::format(BX_SEND_TX_OUTPUT) % now());
    state.stop();
}

console_result send_tx::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& transactions = get_transactions_argument();

    callback_state state(error, output);
    const auto handler = [&state](const std::error_code& code)
    {
        if (!state.handle_error(code))
            handle_callback(state);
    };

    obelisk_client client(*this);
    auto& fullnode = client.get_fullnode();
    for (const tx_type& tx: transactions)
    {
        ++state;
        fullnode.protocol.broadcast_transaction(tx, handler);
    }

    client.poll(state.stopped());

    return state.get_result();
}

