# SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: CC0-1.0
import pytest
from pytest_embedded import Dut
from pytest_embedded_idf.utils import idf_parametrize

@pytest.mark.generic
@idf_parametrize('target', ['supported_targets'], indirect=['target'])
def test_unit_test_app(dut: Dut) -> None:
    def get_reg_nums(number: int) -> str:
        return r'\d{1,2}\s+' * number

    # dut.expect_exact('In main application. Collecting 32 random numbers from 1 to 100:')
    # dut.expect(get_reg_nums(10))
    # dut.expect(get_reg_nums(10))
    # dut.expect(get_reg_nums(10))
    # dut.expect(get_reg_nums(2))

    print('this would be redirected')

    dut.expect('this')
    dut.expect_exact('would')
    dut.expect('[be]{2}')
    dut.expect_exact('redirected')



# def test_basic_expect(redirect, dut: Dut):
#     with redirect():
#         print('this would be redirected')

#     dut.expect('this')
#     dut.expect_exact('would')
#     dut.expect('[be]{2}')
#     dut.expect_exact('redirected')